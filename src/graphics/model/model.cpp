// std
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <functional>
#include <future>
#include <memory>

// internal
#include <core/allocator/global_allocator.hpp>
#include <core/buffer/buffer.hpp>
#include <core/core.hpp>
#include <core/data/ptr.hpp>
#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>
#include <graphics/graphics.hpp>
#include <graphics/model/model.hpp>
#include <math/geometry/skinned_vertex.hpp>
#include <math/geometry/vertex.hpp>
#include <math/math.hpp>
#include <platform/io/asset_io.hpp>
#include <script/luaapi.hpp>

#include <assimp/IOSystem.hpp>
#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <math/matrix.hpp>
#include <math/quaternion.hpp>
#include <math/vector.hpp>

namespace sinen {
std::pair<Ptr<gpu::Buffer>, Ptr<gpu::Buffer>>
createVertexIndexBuffer(const Array<Vertex> &vertices,
                        const Array<uint32_t> &indices);
Ptr<gpu::Buffer>
createSkinnedVertexBuffer(const Array<SkinnedVertex> &vertices);
Ptr<gpu::Buffer> createBuffer(size_t size, void *data, gpu::BufferUsage usage);

namespace {
struct EmbeddedTextureData {
  bool present = false;
  bool compressed = false;
  uint32_t width = 0;
  uint32_t height = 0;
  int channels = 4;
  gpu::TextureFormat format = gpu::TextureFormat::R8G8B8A8_UNORM;
  Array<char> bytes;
};

static EmbeddedTextureData extractEmbeddedTexture(aiScene *scene,
                                                  aiMaterial *material,
                                                  aiTextureType type) {
  EmbeddedTextureData out;

  aiString texPath;
  if (material->GetTexture(type, 0, &texPath) != AI_SUCCESS) {
    return out;
  }

  auto *aiTex = scene->GetEmbeddedTexture(texPath.C_Str());
  if (!aiTex) {
    return out;
  }

  out.present = true;
  if (aiTex->mHeight == 0) {
    out.compressed = true;
    out.bytes.resize(aiTex->mWidth);
    std::memcpy(out.bytes.data(), aiTex->pcData, aiTex->mWidth);
    return out;
  }

  out.compressed = false;
  out.width = aiTex->mWidth;
  out.height = aiTex->mHeight;
  out.channels = 4;
  out.format = gpu::TextureFormat::R8G8B8A8_UNORM;

  const size_t sizeBytes =
      static_cast<size_t>(out.width) * static_cast<size_t>(out.height) * 4u;
  out.bytes.resize(sizeBytes);
  std::memcpy(out.bytes.data(), aiTex->pcData, sizeBytes);
  return out;
}

struct AsyncModelState {
  std::future<void> future;
  Mesh mesh;
  AABB aabb;
  Model::BoneMap boneMap;
  SkeletalAnimation skeletalAnimation;
  Array<EmbeddedTextureData> embeddedTextures;
  bool ok = false;
  std::atomic<bool> finalized = false;
};
} // namespace

enum class LoadState { Version, Vertex, Indices };
Array<Vec3> getVector3sFromKey(const aiVectorKey *keys, uint32_t count) {
  Array<Vec3> result;
  for (uint32_t i = 0; i < count; ++i) {
    result.push_back(
        Vec3(keys[i].mValue.x, keys[i].mValue.y, keys[i].mValue.z));
  }
  return result;
};
Array<float> getTimesFromVector3Key(const aiVectorKey *keys, uint32_t count) {
  Array<float> result;
  for (uint32_t i = 0; i < count; ++i) {
    result.push_back(static_cast<float>(keys[i].mTime));
  }
  return result;
};
Array<Quat> getRotatesFromKey(const aiQuatKey *keys, uint32_t count) {
  Array<Quat> result;
  for (uint32_t i = 0; i < count; ++i) {
    result.push_back(Quat(keys[i].mValue.x, keys[i].mValue.y, keys[i].mValue.z,
                          keys[i].mValue.w));
  }
  return result;
};
Array<float> getTimesFromQuatKey(const aiQuatKey *keys, uint32_t count) {
  Array<float> result;
  for (uint32_t i = 0; i < count; ++i) {
    result.push_back(static_cast<float>(keys[i].mTime));
  }
  return result;
};
Mat4 convertMatrix(const aiMatrix4x4 &m) { return (Mat4(&m.a1)); }

Node createNodeFromAiNode(const aiNode *ainode) {
  Node node;
  node.name = ainode->mName.C_Str();
  node.transformation = convertMatrix(ainode->mTransformation);
  for (uint32_t i = 0; i < ainode->mNumChildren; ++i) {
    node.children.push_back(createNodeFromAiNode(ainode->mChildren[i]));
  }
  return node;
}

void loadAnimation(const aiScene *scene, SkeletalAnimation &skeletalAnimation,
                   const Model::BoneMap &boneMap) {
  if (scene->HasAnimations()) {
    {

      auto anim = scene->mAnimations[0];
      Hashmap<String, NodeAnimation> nodeAnimMap;
      for (uint32_t i = 0; i < anim->mNumChannels; ++i) {
        aiNodeAnim *channel = anim->mChannels[i];
        NodeAnimation nodeAnim;
        nodeAnim.position = getVector3sFromKey(channel->mPositionKeys,
                                               channel->mNumPositionKeys);
        nodeAnim.positionTime = getTimesFromVector3Key(
            channel->mPositionKeys, channel->mNumPositionKeys);
        nodeAnim.rotation = getRotatesFromKey(channel->mRotationKeys,
                                              channel->mNumRotationKeys);
        nodeAnim.rotationTime = getTimesFromQuatKey(channel->mRotationKeys,
                                                    channel->mNumRotationKeys);
        nodeAnim.scale =
            getVector3sFromKey(channel->mScalingKeys, channel->mNumScalingKeys);
        nodeAnim.scaleTime = getTimesFromVector3Key(channel->mScalingKeys,
                                                    channel->mNumScalingKeys);
        nodeAnimMap[channel->mNodeName.C_Str()] = nodeAnim;
      }
      skeletalAnimation.load(createNodeFromAiNode(scene->mRootNode),
                             anim->mTicksPerSecond, anim->mDuration,
                             nodeAnimMap);
    }
    struct BoneData {
      BoneData() : ids(), weights() {}
      Array<uint32_t> ids;
      Array<float> weights;
    };
    for (int i = 0; i < scene->mNumMeshes; i++) {
      const aiMesh *aimesh = scene->mMeshes[i];
      Hashmap<uint32_t, BoneData> boneData;

      for (uint32_t j = 0; j < aimesh->mNumBones; j++) {
        aiBone *bone = aimesh->mBones[j];
        String boneName = bone->mName.C_Str();

        uint32_t index = boneMap.at(boneName).index;

        for (uint32_t k = 0; k < bone->mNumWeights; ++k) {
          uint32_t vertexId = bone->mWeights[k].mVertexId;
          float weight = bone->mWeights[k].mWeight;
          boneData[vertexId].ids.push_back(index);
          boneData[vertexId].weights.push_back(weight);
        }
      }

      for (uint32_t j = 0; j < aimesh->mNumVertices; ++j) {
        SkinnedVertex animationVertex;
        const auto &ids = boneData[j].ids;
        const auto &ws = boneData[j].weights;
        float temp = 0.f;
        assert(ids.size() <= 4);
        assert(ws.size() <= 4);
        for (int k = 0; k < 4; ++k) {
          animationVertex.boneIDs[k] = (k < ids.size()) ? float(ids[k]) : 0.0f;
          animationVertex.boneWeights[k] = (k < ws.size()) ? ws[k] : 0.0f;
        }
        skeletalAnimation.skinnedVertices.push_back(animationVertex);
      }
    }
  }
}

void loadBone(const aiScene *scene, Model::BoneMap &boneMap) {
  for (int i = 0; i < scene->mNumMeshes; i++) {
    const aiMesh *aimesh = scene->mMeshes[i];
    for (uint32_t j = 0; j < aimesh->mNumBones; j++) {
      aiBone *bone = aimesh->mBones[j];
      String boneName = bone->mName.C_Str();

      if (!boneMap.contains(boneName)) {
        Model::BoneInfo boneInfo;
        boneInfo.index = static_cast<uint32_t>(boneMap.size());
        boneInfo.offsetMatrix = convertMatrix(bone->mOffsetMatrix);
        boneMap[boneName] = boneInfo;
      }
    }
  }
}

void loadMesh(const aiScene *scene, Mesh &mesh, AABB &aabb) {
  if (scene->HasMeshes()) {
    uint32_t baseIndex = 0;
    // Iterate through the meshes
    for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
      bool hasBaseColor = false;
      const aiMesh *aimesh = scene->mMeshes[i];
      aiColor4D aiColor;
      if (scene->HasMaterials()) {
        auto result = scene->mMaterials[aimesh->mMaterialIndex]->Get(
            AI_MATKEY_BASE_COLOR, aiColor);
        hasBaseColor = result == aiReturn_SUCCESS;
      }
      // Process the mesh data
      for (uint32_t j = 0; j < aimesh->mNumVertices; j++) {
        const aiVector3D &pos = aimesh->mVertices[j];
        const aiVector3D &norm = aimesh->mNormals[j];
        Vec2 uv(0.f, 0.f);
        if (aimesh->HasTextureCoords(0)) {
          const aiVector3D &aiuv = aimesh->mTextureCoords[0][j];
          uv.x = aiuv.x;
          uv.y = aiuv.y;
        }
        Color color = Color(1.f, 1.f, 1.f, 1.f);
        if (aimesh->HasVertexColors(j)) {
          const aiColor4D &c = aimesh->mColors[0][j];
          color = Color(c.r, c.g, c.b, c.a);
        } else if (hasBaseColor) {
          color.r = aiColor.r;
          color.g = aiColor.g;
          color.b = aiColor.b;
          color.a = aiColor.a;
        }

        Vertex v;
        v.position.x = pos.x;
        v.position.y = pos.y;
        v.position.z = pos.z;
        v.normal.x = norm.x;
        v.normal.y = norm.y;
        v.normal.z = norm.z;
        v.uv.x = uv.x;
        v.uv.y = 1.f - uv.y;
        v.color = color;

        aabb.min.x = Math::min(aabb.min.x, static_cast<float>(pos.x));
        aabb.min.y = Math::min(aabb.min.y, static_cast<float>(pos.y));
        aabb.min.z = Math::min(aabb.min.z, static_cast<float>(pos.z));
        aabb.max.x = Math::max(aabb.max.x, static_cast<float>(pos.x));
        aabb.max.y = Math::max(aabb.max.y, static_cast<float>(pos.y));
        aabb.max.z = Math::max(aabb.max.z, static_cast<float>(pos.z));

        mesh.data()->vertices.push_back(v);
      }
      // Process the indices
      uint32_t maxIndex = 0;
      for (uint32_t j = 0; j < aimesh->mNumFaces; j++) {
        const aiFace &face = aimesh->mFaces[j];
        for (uint32_t k = 0; k < face.mNumIndices; k++) {
          uint32_t index = face.mIndices[k];
          maxIndex = Math::max(maxIndex, index);
          mesh.data()->indices.push_back(baseIndex + index);
        }
      }
      baseIndex += aimesh->mNumVertices;
    }
  }
}

void calcTangents(const aiScene *scene, Mesh &m) {
  auto mesh = m.data();
  auto vertexCount = mesh->vertices.size();
  mesh->tangents.resize(vertexCount, Vec4(0.0f));

  Array<Vec3> tangentAccum(vertexCount, Vec3(0.0f));
  Array<float> handedness(vertexCount, 1.0f);

  for (size_t i = 0; i < mesh->indices.size(); i += 3) {
    Vertex &v0 = mesh->vertices[mesh->indices[i]];
    Vertex &v1 = mesh->vertices[mesh->indices[i + 1]];
    Vertex &v2 = mesh->vertices[mesh->indices[i + 2]];

    Vec3 edge1 = v1.position - v0.position;
    Vec3 edge2 = v2.position - v0.position;
    Vec2 deltaUV1 = v1.uv - v0.uv;
    Vec2 deltaUV2 = v2.uv - v0.uv;

    float det = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
    if (fabs(det) < 1e-6f)
      continue;
    float f = 1.0f / det;

    Vec3 tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * f;
    Vec3 bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * f;

    tangentAccum[mesh->indices[i]] += tangent;
    tangentAccum[mesh->indices[i + 1]] += tangent;
    tangentAccum[mesh->indices[i + 2]] += tangent;

    // optional: handedness check
    Vec3 N = Vec3::normalize(v0.normal);
    float sign =
        (Vec3::dot(Vec3::cross(N, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;
    handedness[mesh->indices[i]] = sign;
    handedness[mesh->indices[i + 1]] = sign;
    handedness[mesh->indices[i + 2]] = sign;
  }

  for (size_t j = 0; j < mesh->vertices.size(); ++j) {
    auto T = Vec3::normalize(tangentAccum[j]);
    mesh->tangents[j] = Vec4(T, handedness[j]);
  }
}

Ptr<Texture> loadTexture(aiScene *scene, aiMaterial *material,
                         aiTextureType type) {
  aiString texPath;
  if (material->GetTexture(type, 0, &texPath) != AI_SUCCESS) {
    return nullptr;
  }
  auto *aiTex = scene->GetEmbeddedTexture(texPath.C_Str());
  if (!aiTex) {
    return nullptr;
  }
  auto texture = Texture::create();
  Array<char> buffer;
  if (aiTex->mHeight == 0) {
    buffer.resize(aiTex->mWidth);
    memcpy(buffer.data(), aiTex->pcData, aiTex->mWidth);
    texture->loadFromMemory(buffer);
  } else {
    texture->loadFromMemory(aiTex->pcData, aiTex->mWidth, aiTex->mHeight,
                            gpu::TextureFormat::R8G8B8A8_UNORM, 4);
  }
  return texture;
}
void loadMaterial(aiScene *scene, Array<Ptr<Texture>> &textures) {
  for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
    if (!textures[0]) {
      textures[0] =
          loadTexture(scene, scene->mMaterials[i], aiTextureType_BASE_COLOR);
    }
    if (!textures[1]) {
      textures[1] =
          loadTexture(scene, scene->mMaterials[i], aiTextureType_NORMALS);
    }
    if (!textures[2]) {
      textures[2] = loadTexture(scene, scene->mMaterials[i],
                                aiTextureType_DIFFUSE_ROUGHNESS);
    }
    if (!textures[3]) {
      textures[3] =
          loadTexture(scene, scene->mMaterials[i], aiTextureType_METALNESS);
    }
    if (!textures[4]) {
      textures[4] =
          loadTexture(scene, scene->mMaterials[i], aiTextureType_EMISSIVE);
    }
    if (!textures[5]) {
      textures[5] =
          loadTexture(scene, scene->mMaterials[i], aiTextureType_LIGHTMAP);
    }
  }
}
Model::Model() : textures(6) {}

void Model::load(StringView path) {
  const TaskGroup group = LoadContext::current();
  group.add();

  this->vertexBuffer = nullptr;
  this->tangentBuffer = nullptr;
  this->animationVertexBuffer = nullptr;
  this->indexBuffer = nullptr;
  this->textures.assign(6, nullptr);

  auto state = makePtr<AsyncModelState>();
  state->embeddedTextures.resize(6);
  const Ptr<void> stateVoid = std::static_pointer_cast<void>(state);
  this->data = stateVoid;

  const String pathStr = path.data();
  state->future = globalThreadPool().submit([state, pathStr]() {
    auto fullFilePath = AssetIO::getFilePath(pathStr);

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        fullFilePath.c_str(),
        aiProcess_ValidateDataStructure | aiProcess_LimitBoneWeights |
            aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);
    if (!scene) {
      state->ok = false;
      return;
    }

    Mesh mesh;
    AABB aabb;
    Model::BoneMap boneMap;
    SkeletalAnimation skeletalAnimation;

    loadBone(scene, boneMap);
    loadAnimation(scene, skeletalAnimation, boneMap);
    loadMesh(scene, mesh, aabb);
    calcTangents(scene, mesh);

    Array<EmbeddedTextureData> embedded;
    embedded.resize(6);
    for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
      if (!embedded[0].present) {
        embedded[0] = extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                             scene->mMaterials[i],
                                             aiTextureType_BASE_COLOR);
      }
      if (!embedded[1].present) {
        embedded[1] =
            extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                   scene->mMaterials[i], aiTextureType_NORMALS);
      }
      if (!embedded[2].present) {
        embedded[2] = extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                             scene->mMaterials[i],
                                             aiTextureType_DIFFUSE_ROUGHNESS);
      }
      if (!embedded[3].present) {
        embedded[3] = extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                             scene->mMaterials[i],
                                             aiTextureType_METALNESS);
      }
      if (!embedded[4].present) {
        embedded[4] = extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                             scene->mMaterials[i],
                                             aiTextureType_EMISSIVE);
      }
      if (!embedded[5].present) {
        embedded[5] = extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                             scene->mMaterials[i],
                                             aiTextureType_LIGHTMAP);
      }
    }

    state->mesh = std::move(mesh);
    state->aabb = aabb;
    state->boneMap = std::move(boneMap);
    state->skeletalAnimation = std::move(skeletalAnimation);
    state->embeddedTextures = std::move(embedded);
    state->ok = true;
  });

  auto pollAndFinalize = std::make_shared<std::function<void()>>();
  *pollAndFinalize = [this, pollAndFinalize, state, stateVoid, group]() {
    if (!state->future.valid()) {
      group.done();
      return;
    }
    if (state->future.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
      Graphics::addPreDrawFunc(*pollAndFinalize);
      return;
    }
    if (state->finalized.exchange(true)) {
      return;
    }

    state->future.get();

    if (this->data != stateVoid) {
      group.done();
      return;
    }

    if (!state->ok) {
      group.done();
      return;
    }

    this->mesh = std::move(state->mesh);
    this->localAABB = state->aabb;
    this->boneMap = std::move(state->boneMap);
    this->skeletalAnimation = std::move(state->skeletalAnimation);
    this->skeletalAnimation.owner = this;

    {
      ScopedLoadContext ctx(group);
      for (size_t i = 0; i < state->embeddedTextures.size() && i < 6; ++i) {
        const auto &d = state->embeddedTextures[i];
        if (!d.present) {
          continue;
        }
        auto texture = Texture::create();
        if (d.compressed) {
          auto bytes = d.bytes;
          texture->loadFromMemory(bytes);
        } else {
          texture->loadFromMemory(const_cast<char *>(d.bytes.data()), d.width,
                                  d.height, d.format, d.channels);
        }
        this->textures[i] = texture;
      }
    }

    auto mesh = this->mesh.data();
    this->vertexBuffer =
        createBuffer(mesh->vertices.size() * sizeof(Vertex),
                     mesh->vertices.data(), gpu::BufferUsage::Vertex);
    this->animationVertexBuffer =
        createSkinnedVertexBuffer(this->skeletalAnimation.skinnedVertices);
    this->tangentBuffer =
        createBuffer(mesh->tangents.size() * sizeof(Vec4),
                     mesh->tangents.data(), gpu::BufferUsage::Vertex);
    this->indexBuffer =
        createBuffer(mesh->indices.size() * sizeof(uint32_t),
                     mesh->indices.data(), gpu::BufferUsage::Index);

    this->data.reset();
    group.done();
  };
  Graphics::addPreDrawFunc(*pollAndFinalize);
}
void Model::load(const Buffer &buffer) {
  const TaskGroup group = LoadContext::current();
  group.add();

  this->vertexBuffer = nullptr;
  this->tangentBuffer = nullptr;
  this->animationVertexBuffer = nullptr;
  this->indexBuffer = nullptr;
  this->textures.assign(6, nullptr);

  auto state = makePtr<AsyncModelState>();
  state->embeddedTextures.resize(6);
  const Ptr<void> stateVoid = std::static_pointer_cast<void>(state);
  this->data = stateVoid;

  const Buffer buf = buffer;
  state->future = globalThreadPool().submit([state, buf]() {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFileFromMemory(
        buf.data(), buf.size(),
        aiProcess_ValidateDataStructure | aiProcess_LimitBoneWeights |
            aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);
    if (!scene) {
      state->ok = false;
      return;
    }

    Mesh mesh;
    AABB aabb;
    Model::BoneMap boneMap;
    SkeletalAnimation skeletalAnimation;

    loadBone(scene, boneMap);
    loadAnimation(scene, skeletalAnimation, boneMap);
    loadMesh(scene, mesh, aabb);
    calcTangents(scene, mesh);

    Array<EmbeddedTextureData> embedded;
    embedded.resize(6);
    for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
      if (!embedded[0].present) {
        embedded[0] = extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                             scene->mMaterials[i],
                                             aiTextureType_BASE_COLOR);
      }
      if (!embedded[1].present) {
        embedded[1] =
            extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                   scene->mMaterials[i], aiTextureType_NORMALS);
      }
      if (!embedded[2].present) {
        embedded[2] = extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                             scene->mMaterials[i],
                                             aiTextureType_DIFFUSE_ROUGHNESS);
      }
      if (!embedded[3].present) {
        embedded[3] = extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                             scene->mMaterials[i],
                                             aiTextureType_METALNESS);
      }
      if (!embedded[4].present) {
        embedded[4] = extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                             scene->mMaterials[i],
                                             aiTextureType_EMISSIVE);
      }
      if (!embedded[5].present) {
        embedded[5] = extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                             scene->mMaterials[i],
                                             aiTextureType_LIGHTMAP);
      }
    }

    state->mesh = std::move(mesh);
    state->aabb = aabb;
    state->boneMap = std::move(boneMap);
    state->skeletalAnimation = std::move(skeletalAnimation);
    state->embeddedTextures = std::move(embedded);
    state->ok = true;
  });

  auto pollAndFinalize = std::make_shared<std::function<void()>>();
  *pollAndFinalize = [this, pollAndFinalize, state, stateVoid, group]() {
    if (!state->future.valid()) {
      group.done();
      return;
    }
    if (state->future.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
      Graphics::addPreDrawFunc(*pollAndFinalize);
      return;
    }
    if (state->finalized.exchange(true)) {
      return;
    }

    state->future.get();

    if (this->data != stateVoid) {
      group.done();
      return;
    }

    if (!state->ok) {
      group.done();
      return;
    }

    this->mesh = std::move(state->mesh);
    this->localAABB = state->aabb;
    this->boneMap = std::move(state->boneMap);
    this->skeletalAnimation = std::move(state->skeletalAnimation);
    this->skeletalAnimation.owner = this;

    {
      ScopedLoadContext ctx(group);
      for (size_t i = 0; i < state->embeddedTextures.size() && i < 6; ++i) {
        const auto &d = state->embeddedTextures[i];
        if (!d.present) {
          continue;
        }
        auto texture = Texture::create();
        if (d.compressed) {
          auto bytes = d.bytes;
          texture->loadFromMemory(bytes);
        } else {
          texture->loadFromMemory(const_cast<char *>(d.bytes.data()), d.width,
                                  d.height, d.format, d.channels);
        }
        this->textures[i] = texture;
      }
    }

    auto mesh = this->mesh.data();
    this->vertexBuffer =
        createBuffer(mesh->vertices.size() * sizeof(Vertex),
                     mesh->vertices.data(), gpu::BufferUsage::Vertex);
    this->animationVertexBuffer =
        createSkinnedVertexBuffer(this->skeletalAnimation.skinnedVertices);
    this->tangentBuffer =
        createBuffer(mesh->tangents.size() * sizeof(Vec4),
                     mesh->tangents.data(), gpu::BufferUsage::Vertex);
    this->indexBuffer =
        createBuffer(mesh->indices.size() * sizeof(uint32_t),
                     mesh->indices.data(), gpu::BufferUsage::Index);

    this->data.reset();
    group.done();
  };
  Graphics::addPreDrawFunc(*pollAndFinalize);
}

void Model::loadFromVertexArray(const Mesh &m) {
  this->mesh = m;
  auto mesh = this->mesh.data();
  AABB aabb;
  for (auto &v : mesh->vertices) {
    aabb.min.x = Math::min(aabb.min.x, v.position.x);
    aabb.min.y = Math::min(aabb.min.y, v.position.y);
    aabb.min.z = Math::min(aabb.min.z, v.position.z);
    aabb.max.x = Math::max(aabb.max.x, v.position.x);
    aabb.max.y = Math::max(aabb.max.y, v.position.y);
    aabb.max.z = Math::max(aabb.max.z, v.position.z);
  }
  this->localAABB = aabb;
  auto viBuffer = createVertexIndexBuffer(mesh->vertices, mesh->indices);
  this->vertexBuffer = viBuffer.first;
  this->indexBuffer = viBuffer.second;
}

void Model::loadSprite() {
  Rect rect;
  loadFromVertexArray(rect.createMesh());
}
void Model::loadBox() {
  AABB aabb;
  loadFromVertexArray(aabb.createMesh());
}

const AABB &Model::getAABB() const { return this->localAABB; }

std::pair<Ptr<gpu::Buffer>, Ptr<gpu::Buffer>>
createVertexIndexBuffer(const Array<Vertex> &vertices,
                        const Array<uint32_t> &indices) {
  auto device = Graphics::getDevice();
  size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
  Ptr<gpu::Buffer> vertexBuffer, indexBuffer;
  gpu::Buffer::CreateInfo vertexBufferInfo{};
  vertexBufferInfo.allocator = GlobalAllocator::get();
  vertexBufferInfo.size = vertexBufferSize;
  vertexBufferInfo.usage = gpu::BufferUsage::Vertex;
  vertexBuffer = device->createBuffer(vertexBufferInfo);

  gpu::Buffer::CreateInfo indexBufferInfo{};
  indexBufferInfo.allocator = GlobalAllocator::get();
  indexBufferInfo.size = indices.size() * sizeof(uint32_t);
  indexBufferInfo.usage = gpu::BufferUsage::Index;
  indexBuffer = device->createBuffer(indexBufferInfo);

  Ptr<gpu::TransferBuffer> transferBuffer;
  {
    {
      gpu::TransferBuffer::CreateInfo info{};
      info.allocator = GlobalAllocator::get();
      info.size = vertexBufferSize;
      info.usage = gpu::TransferBufferUsage::Upload;
      transferBuffer = device->createTransferBuffer(info);
      auto *pMapped = transferBuffer->map(false);
      memcpy(pMapped, vertices.data(), vertices.size() * sizeof(Vertex));
      transferBuffer->unmap();
    }
    {
      gpu::CommandBuffer::CreateInfo info{};
      info.allocator = GlobalAllocator::get();
      auto commandBuffer = device->acquireCommandBuffer(info);
      {

        auto copyPass = commandBuffer->beginCopyPass();
        {

          gpu::BufferTransferInfo src{};
          src.offset = 0;
          src.transferBuffer = transferBuffer;
          gpu::BufferRegion dst{};
          dst.offset = 0;
          dst.size = vertexBufferSize;
          dst.buffer = vertexBuffer;
          copyPass->uploadBuffer(src, dst, false);
        }
        commandBuffer->endCopyPass(copyPass);
      }
      device->submitCommandBuffer(commandBuffer);
    }
  }
  {
    gpu::TransferBuffer::CreateInfo info{};
    info.allocator = GlobalAllocator::get();
    info.size = indexBufferInfo.size;
    info.usage = gpu::TransferBufferUsage::Upload;
    transferBuffer = device->createTransferBuffer(info);
    auto *pMapped = transferBuffer->map(false);
    memcpy(pMapped, indices.data(), indexBufferInfo.size);
    transferBuffer->unmap();
  }
  {
    {
      gpu::CommandBuffer::CreateInfo info{};
      info.allocator = GlobalAllocator::get();
      auto commandBuffer = device->acquireCommandBuffer(info);
      {

        auto copyPass = commandBuffer->beginCopyPass();
        {

          gpu::BufferTransferInfo src{};
          src.offset = 0;
          src.transferBuffer = transferBuffer;
          gpu::BufferRegion dst{};
          dst.offset = 0;
          dst.size = indexBufferInfo.size;
          dst.buffer = indexBuffer;
          copyPass->uploadBuffer(src, dst, false);
        }
        commandBuffer->endCopyPass(copyPass);
      }
      device->submitCommandBuffer(commandBuffer);
    }
  }
  return std::make_pair(vertexBuffer, indexBuffer);
}
Ptr<gpu::Buffer>
createSkinnedVertexBuffer(const Array<SkinnedVertex> &vertices) {
  if (vertices.empty())
    return nullptr;
  auto device = Graphics::getDevice();
  size_t vertexBufferSize = vertices.size() * sizeof(SkinnedVertex);
  Ptr<gpu::Buffer> vertexBuffer;
  gpu::Buffer::CreateInfo vertexBufferInfo{};
  vertexBufferInfo.allocator = GlobalAllocator::get();
  vertexBufferInfo.size = vertexBufferSize;
  vertexBufferInfo.usage = gpu::BufferUsage::Vertex;
  vertexBuffer = device->createBuffer(vertexBufferInfo);

  Ptr<gpu::TransferBuffer> transferBuffer;
  {
    {
      gpu::TransferBuffer::CreateInfo info{};
      info.allocator = GlobalAllocator::get();
      info.size = vertexBufferSize;
      info.usage = gpu::TransferBufferUsage::Upload;
      transferBuffer = device->createTransferBuffer(info);
      auto *pMapped = transferBuffer->map(false);
      memcpy(pMapped, vertices.data(), vertices.size() * sizeof(SkinnedVertex));
      transferBuffer->unmap();
    }
    {
      gpu::CommandBuffer::CreateInfo info{};
      info.allocator = GlobalAllocator::get();
      auto commandBuffer = device->acquireCommandBuffer(info);
      {

        auto copyPass = commandBuffer->beginCopyPass();
        {

          gpu::BufferTransferInfo src{};
          src.offset = 0;
          src.transferBuffer = transferBuffer;
          gpu::BufferRegion dst{};
          dst.offset = 0;
          dst.size = vertexBufferSize;
          dst.buffer = vertexBuffer;
          copyPass->uploadBuffer(src, dst, false);
        }
        commandBuffer->endCopyPass(copyPass);
      }
      device->submitCommandBuffer(commandBuffer);
    }
  }
  return vertexBuffer;
}
Ptr<gpu::Buffer> createBuffer(size_t size, void *data, gpu::BufferUsage usage) {

  if (!data)
    return nullptr;
  auto device = Graphics::getDevice();
  Ptr<gpu::Buffer> buffer;
  gpu::Buffer::CreateInfo vertexBufferInfo{};
  vertexBufferInfo.allocator = GlobalAllocator::get();
  vertexBufferInfo.size = size;
  vertexBufferInfo.usage = usage;
  buffer = device->createBuffer(vertexBufferInfo);

  Ptr<gpu::TransferBuffer> transferBuffer;
  {
    {
      gpu::TransferBuffer::CreateInfo info{};
      info.allocator = GlobalAllocator::get();
      info.size = size;
      info.usage = gpu::TransferBufferUsage::Upload;
      transferBuffer = device->createTransferBuffer(info);
      auto *pMapped = transferBuffer->map(false);
      memcpy(pMapped, data, size);
      transferBuffer->unmap();
    }
    {
      gpu::CommandBuffer::CreateInfo info{};
      info.allocator = GlobalAllocator::get();
      auto commandBuffer = device->acquireCommandBuffer(info);
      {

        auto copyPass = commandBuffer->beginCopyPass();
        {

          gpu::BufferTransferInfo src{};
          src.offset = 0;
          src.transferBuffer = transferBuffer;
          gpu::BufferRegion dst{};
          dst.offset = 0;
          dst.size = size;
          dst.buffer = buffer;
          copyPass->uploadBuffer(src, dst, false);
        }
        commandBuffer->endCopyPass(copyPass);
      }
      device->submitCommandBuffer(commandBuffer);
    }
  }
  return buffer;
}

Buffer Model::getBoneUniformBuffer() const {
  auto size = boneMatrices.size() * sizeof(Mat4);
  auto *ptr = (Mat4 *)GlobalAllocator::get()->allocate(size);
  memcpy(ptr, boneMatrices.data(), size);
  auto deleter = Deleter<void>(GlobalAllocator::get(), size);
  return Buffer(BufferType::Binary, Ptr<void>(ptr, std::move(deleter)), size);
}
void Model::play(float start) {
  time = start;
  loadBoneUniform(time);
}
void Model::update(float delta_time) {
  time += delta_time;
  loadBoneUniform(time);
}

void Model::loadBoneUniform(float start) {
  for (auto &bone : boneMap) {
    assert(bone.second.index <= boneMap.size());
  }
  skeletalAnimation.update(start);
  for (auto &bone : boneMap) {
    assert(bone.second.index <= boneMap.size());
  }
  auto matrices = skeletalAnimation.getFinalBoneMatrices();

  boneMatrices.clear();
  boneMatrices.resize(matrices.size());
  int index = 0;
  for (auto &m : matrices) {
    boneMatrices[index] = m;
    index++;
  }
}
bool Model::hasTexture(TextureKey type) const {
  return textures[static_cast<UInt32>(type)] != nullptr;
}
Ptr<Texture> Model::getTexture(TextureKey type) const {
  return textures[static_cast<UInt32>(type)];
}
void Model::setTexture(TextureKey type, const Ptr<Texture> &texture) {
  textures[static_cast<UInt32>(type)] = texture;
}

static int lModelNew(lua_State *L) {
  udPushPtr<Model>(L, makePtr<Model>());
  return 1;
}
static int lModelLoad(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  if (lua_isstring(L, 2)) {
    const char *path = luaL_checkstring(L, 2);
    m->load(StringView(path));
    return 0;
  }
  auto &buf = udValue<Buffer>(L, 2);
  m->load(buf);
  return 0;
}
static int lModelGetAabb(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  udNewOwned<AABB>(L, m->getAABB());
  return 1;
}
static int lModelLoadSprite(lua_State *L) {
  udPtr<Model>(L, 1)->loadSprite();
  return 0;
}
static int lModelLoadBox(lua_State *L) {
  udPtr<Model>(L, 1)->loadBox();
  return 0;
}
static int lModelGetBoneUniformBuffer(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  udNewOwned<Buffer>(L, m->getBoneUniformBuffer());
  return 1;
}
static int lModelPlay(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  float start = static_cast<float>(luaL_checknumber(L, 2));
  m->play(start);
  return 0;
}
static int lModelUpdate(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  float dt = static_cast<float>(luaL_checknumber(L, 2));
  m->update(dt);
  return 0;
}
static int lModelHasTexture(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  auto k = static_cast<TextureKey>(luaL_checkinteger(L, 2));
  lua_pushboolean(L, m->hasTexture(k));
  return 1;
}
static int lModelGetTexture(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  auto k = static_cast<TextureKey>(luaL_checkinteger(L, 2));
  udPushPtr<Texture>(L, m->getTexture(k));
  return 1;
}
static int lModelSetTexture(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  auto k = static_cast<TextureKey>(luaL_checkinteger(L, 2));
  auto &t = udPtr<Texture>(L, 3);
  m->setTexture(k, t);
  return 0;
}
void registerModel(lua_State *L) {
  luaL_newmetatable(L, Model::metaTableName());
  luaPushcfunction2(L, udPtrGc<Model>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lModelLoad);
  lua_setfield(L, -2, "load");
  luaPushcfunction2(L, lModelGetAabb);
  lua_setfield(L, -2, "getAABB");
  luaPushcfunction2(L, lModelLoadSprite);
  lua_setfield(L, -2, "loadSprite");
  luaPushcfunction2(L, lModelLoadBox);
  lua_setfield(L, -2, "loadBox");
  luaPushcfunction2(L, lModelGetBoneUniformBuffer);
  lua_setfield(L, -2, "getBoneUniformBuffer");
  luaPushcfunction2(L, lModelPlay);
  lua_setfield(L, -2, "play");
  luaPushcfunction2(L, lModelUpdate);
  lua_setfield(L, -2, "update");
  luaPushcfunction2(L, lModelHasTexture);
  lua_setfield(L, -2, "hasTexture");
  luaPushcfunction2(L, lModelGetTexture);
  lua_setfield(L, -2, "getTexture");
  luaPushcfunction2(L, lModelSetTexture);
  lua_setfield(L, -2, "setTexture");
  lua_pop(L, 1);

  pushSnNamed(L, "Model");
  luaPushcfunction2(L, lModelNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

void registerTextureKey(lua_State *L) {
  pushSnNamed(L, "TextureKey");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::BaseColor));
  lua_setfield(L, -2, "BaseColor");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::Normal));
  lua_setfield(L, -2, "Normal");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::DiffuseRoughness));
  lua_setfield(L, -2, "DiffuseRoughness");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::Metalness));
  lua_setfield(L, -2, "Metalness");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::Emissive));
  lua_setfield(L, -2, "Emissive");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::LightMap));
  lua_setfield(L, -2, "LightMap");
  lua_pop(L, 1);
}
} // namespace sinen
