// std
#include <atomic>
#include <cassert>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <functional>
#include <future>
#include <iterator>
#include <memory>

// internal
#include <core/allocator/global_allocator.hpp>
#include <core/buffer/buffer.hpp>
#include <core/core.hpp>
#include <core/data/ptr.hpp>
#include <core/profiler.hpp>
#include <core/thread/future_poll.hpp>
#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>
#include <graphics/graphics.hpp>
#include <graphics/model/model.hpp>
#include <math/geometry/skinned_vertex.hpp>
#include <math/geometry/vertex.hpp>
#include <math/mat4.hpp>
#include <math/math.hpp>
#include <platform/io/asset_reader.hpp>

#include <assimp/GltfMaterial.h>
#include <assimp/IOSystem.hpp>
#include <assimp/Importer.hpp>
#include <assimp/config.h>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <meshoptimizer.h>

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
Array<Ptr<gpu::Buffer>> createLodIndexBuffers(const Mesh &mesh,
                                              const Ptr<gpu::Buffer> &base);

namespace {
struct EmbeddedTextureData {
  bool present = false;
  bool compressed = false;
  uint32_t width = 0;
  uint32_t height = 0;
  int channels = 4;
  gpu::TextureFormat format = gpu::TextureFormat::R8G8B8A8_UNORM;
  Array<char> bytes;
  String path;
  String key;
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
  out.key = texPath.C_Str();
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
  for (size_t i = 0; i < sizeBytes / 4; ++i) {
    out.bytes[i * 4] = static_cast<char>(aiTex->pcData[i].r);
    out.bytes[i * 4 + 1] = static_cast<char>(aiTex->pcData[i].g);
    out.bytes[i * 4 + 2] = static_cast<char>(aiTex->pcData[i].b);
    out.bytes[i * 4 + 3] = static_cast<char>(aiTex->pcData[i].a);
  }
  return out;
}

struct ImportedMaterial {
  Array<EmbeddedTextureData> textures{6};
  Model::SurfaceProperties surface;
};
struct ImportedPart {
  Mesh mesh;
  UInt32 material = 0;
};
struct AsyncModelState {
  std::future<void> future;
  String debugName;
  Mesh mesh;
  AABB aabb;
  Model::BoneMap boneMap;
  SkeletalAnimation skeletalAnimation;
  Array<EmbeddedTextureData> embeddedTextures;
  bool ok = false;
  std::atomic<bool> finalized = false;
  Array<ImportedPart> parts;
  Array<ImportedMaterial> materials;
  Model::LoadOptions options;
  TaskGroup group;
  bool staticScene = false;
};

static void optimizeIndexOrder(Mesh &mesh, Array<UInt32> &indices) {
  auto data = mesh.data();
  if (indices.empty() || data->vertices.empty()) {
    return;
  }

  Array<UInt32> cacheOptimized(indices.size());
  meshopt_optimizeVertexCache(cacheOptimized.data(), indices.data(),
                              indices.size(), data->vertices.size());
  meshopt_optimizeOverdraw(indices.data(), cacheOptimized.data(),
                           cacheOptimized.size(), &data->vertices[0].position.x,
                           data->vertices.size(), sizeof(Vertex), 1.05f);
}

static size_t triangleIndexCount(size_t count) { return (count / 3) * 3; }

static String assimpHintFromPath(StringView path) {
  const size_t dot = path.find_last_of('.');
  if (dot == StringView::npos || dot + 1 >= path.size()) {
    return "";
  }
  String hint(path.substr(dot + 1));
  std::transform(hint.begin(), hint.end(), hint.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return hint;
}

static void optimizeMesh(Mesh &mesh) {
  ZoneScopedN("Model mesh optimize");
  auto data = mesh.data();
  if (data->indices.size() < 3 || data->vertices.empty()) {
    return;
  }

  optimizeIndexOrder(mesh, data->indices);

  data->lods.clear();
  data->lods.push_back(
      Mesh::Data::Lod{.indices = data->indices, .threshold = 0.0f});

  constexpr float ratios[] = {0.60f, 0.35f, 0.18f};
  constexpr float thresholds[] = {14.0f, 28.0f, 56.0f};
  constexpr float maxError = 1e-2f;

  size_t previousCount = data->indices.size();
  for (size_t i = 0; i < std::size(ratios); ++i) {
    const size_t targetCount = triangleIndexCount(
        static_cast<size_t>(data->indices.size() * ratios[i]));
    if (targetCount < 12 || targetCount >= previousCount) {
      continue;
    }

    Array<UInt32> simplified(data->indices.size());
    float error = 0.0f;
    size_t simplifiedCount = meshopt_simplify(
        simplified.data(), data->indices.data(), data->indices.size(),
        &data->vertices[0].position.x, data->vertices.size(), sizeof(Vertex),
        targetCount, maxError, 0, &error);
    simplifiedCount = triangleIndexCount(simplifiedCount);
    if (simplifiedCount < 12 || simplifiedCount >= previousCount) {
      continue;
    }

    simplified.resize(simplifiedCount);
    optimizeIndexOrder(mesh, simplified);
    data->lods.push_back(Mesh::Data::Lod{.indices = std::move(simplified),
                                         .threshold = thresholds[i],
                                         .error = error});
    previousCount = simplifiedCount;
  }
}
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
Mat4 convertMatrix(const aiMatrix4x4 &m) {
  const float *mat = &m.a1;
  return Mat4(mat);
}

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

        auto boneIt = boneMap.find(boneName);
        if (boneIt == boneMap.end()) {
          continue;
        }
        uint32_t index = boneIt->second.index;

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
        if (aimesh->HasVertexColors(0)) {
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
namespace {
EmbeddedTextureData importedTexture(const aiScene *scene,
                                    const aiMaterial *material,
                                    std::initializer_list<aiTextureType> types,
                                    StringView modelPath) {
  for (const auto type : types) {
    aiString name;
    if (material->GetTexture(type, 0, &name) != AI_SUCCESS)
      continue;
    if (scene->GetEmbeddedTexture(name.C_Str()))
      return extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                    const_cast<aiMaterial *>(material), type);
    String relative(name.C_Str());
    std::replace(relative.begin(), relative.end(), '\\', '/');
    const auto slash = relative.find_last_of('/');
    const String basename =
        slash == String::npos ? relative : relative.substr(slash + 1);
    const auto modelSlash = modelPath.find_last_of("/\\");
    const String directory = modelSlash == StringView::npos
                                 ? String()
                                 : String(modelPath.substr(0, modelSlash + 1));
    Array<String> candidates;
    if (!relative.empty() && relative.front() != '/' &&
        relative.find(':') == String::npos)
      candidates.push_back(directory + relative);
    candidates.push_back(directory + basename);
    candidates.push_back(directory + "Textures/" + basename);
    candidates.push_back(directory + "textures/" + basename);
    for (const auto &path : candidates)
      if (AssetReader::exists(path)) {
        EmbeddedTextureData result;
        result.present = true;
        result.path = path;
        result.key = path;
        return result;
      }
    Log::warn("Model texture not found: {} (model {})", relative, modelPath);
  }
  return {};
}

bool importStaticScene(const aiScene *scene, AsyncModelState &state,
                       StringView path) {
  if (scene->HasAnimations() && !state.options.staticScene)
    return false;
  for (unsigned i = 0; i < scene->mNumMeshes; ++i)
    if (scene->mMeshes[i]->HasBones())
      return false;
  state.staticScene = true;
  state.materials.resize(scene->mNumMaterials);
  Array<Color> baseColors(scene->mNumMaterials, Color(1.0f));
  for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
    auto &target = state.materials[i];
    auto &surface = target.surface;
    const auto *material = scene->mMaterials[i];
    aiString name;
    material->Get(AI_MATKEY_NAME, name);
    surface.materialName = name.C_Str();
    aiColor4D color(1, 1, 1, 1);
    if (material->Get(AI_MATKEY_BASE_COLOR, color) != AI_SUCCESS)
      material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    float opacity = 1;
    material->Get(AI_MATKEY_OPACITY, opacity);
    baseColors[i] = Color(color.r, color.g, color.b,
                          color.a * std::clamp(opacity, 0.0f, 1.0f));
    surface.metallicFactor = 0;
    material->Get(AI_MATKEY_METALLIC_FACTOR, surface.metallicFactor);
    material->Get(AI_MATKEY_ROUGHNESS_FACTOR, surface.roughnessFactor);
    aiColor3D emission(0, 0, 0);
    material->Get(AI_MATKEY_COLOR_EMISSIVE, emission);
    surface.emissiveFactor = Color(emission.r, emission.g, emission.b, 1);
    surface.normalY = state.options.flipNormalY ? -1.0f : 1.0f;
    surface.autoAlphaMask = state.options.packedSpecular;
    aiString mode;
    if (material->Get(AI_MATKEY_GLTF_ALPHAMODE, mode) == AI_SUCCESS &&
        StringView(mode.C_Str()) == "MASK") {
      surface.alphaCutoff = 0.5f;
      material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, surface.alphaCutoff);
    }
    target.textures[0] = importedTexture(
        scene, material, {aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE},
        path);
    target.textures[1] = importedTexture(
        scene, material, {aiTextureType_NORMALS, aiTextureType_NORMAL_CAMERA},
        path);
    target.textures[2] =
        state.options.packedSpecular
            ? importedTexture(
                  scene, material,
                  {aiTextureType_SPECULAR, aiTextureType_DIFFUSE_ROUGHNESS},
                  path)
            : importedTexture(scene, material,
                              {aiTextureType_DIFFUSE_ROUGHNESS}, path);
    target.textures[3] =
        importedTexture(scene, material, {aiTextureType_METALNESS}, path);
    target.textures[4] =
        importedTexture(scene, material, {aiTextureType_EMISSIVE}, path);
    target.textures[5] = state.options.packedSpecular
                             ? target.textures[2]
                             : importedTexture(scene, material,
                                               {aiTextureType_AMBIENT_OCCLUSION,
                                                aiTextureType_LIGHTMAP},
                                               path);
    if (state.options.packedSpecular) {
      surface.metallicFactor = surface.roughnessFactor = 1;
      if (target.textures[4].present)
        surface.emissiveFactor = Color(1.0f);
    }
  }
  aiMatrix4x4 units;
  // Assimp has already applied the file's FBX axis/unit metadata to its root.
  // Preserve that scale instead of applying a second unit conversion.
  std::function<void(const aiNode *, const aiMatrix4x4 &)> visit =
      [&](const aiNode *node, const aiMatrix4x4 &parent) {
        const auto world = parent * node->mTransformation;
        aiMatrix3x3 normalMatrix(world);
        if (std::abs(normalMatrix.Determinant()) < 1e-12f)
          return;
        const bool mirrored = normalMatrix.Determinant() < 0;
        normalMatrix.Inverse().Transpose();
        for (unsigned slot = 0; slot < node->mNumMeshes; ++slot) {
          const auto *source = scene->mMeshes[node->mMeshes[slot]];
          if (!source->mNumVertices || !source->mNumFaces)
            continue;
          ImportedPart part;
          part.material =
              std::min(source->mMaterialIndex, scene->mNumMaterials - 1);
          const auto base = baseColors[part.material];
          auto mesh = part.mesh.data();
          mesh->vertices.reserve(source->mNumVertices);
          for (unsigned v = 0; v < source->mNumVertices; ++v) {
            const auto p = world * source->mVertices[v];
            auto n =
                normalMatrix * (source->HasNormals() ? source->mNormals[v]
                                                     : aiVector3D(0, 1, 0));
            if (n.SquareLength() > 1e-16f)
              n.Normalize();
            const auto uv = source->HasTextureCoords(0)
                                ? source->mTextureCoords[0][v]
                                : aiVector3D();
            const auto c = source->HasVertexColors(0) ? source->mColors[0][v]
                                                      : aiColor4D(1, 1, 1, 1);
            mesh->vertices.emplace_back(
                Vec3(p.x, p.y, p.z), Vec3(n.x, n.y, n.z), Vec2(uv.x, 1 - uv.y),
                Color(base.r * c.r, base.g * c.g, base.b * c.b, base.a * c.a));
          }
          for (unsigned f = 0; f < source->mNumFaces; ++f) {
            const auto &face = source->mFaces[f];
            if (face.mNumIndices != 3)
              continue;
            mesh->indices.push_back(face.mIndices[0]);
            mesh->indices.push_back(face.mIndices[mirrored ? 2 : 1]);
            mesh->indices.push_back(face.mIndices[mirrored ? 1 : 2]);
          }
          if (mesh->indices.empty())
            continue;
          optimizeMesh(part.mesh);
          state.parts.push_back(std::move(part));
        }
        for (unsigned i = 0; i < node->mNumChildren; ++i)
          visit(node->mChildren[i], world);
      };
  if (scene->mNumMaterials && scene->mRootNode)
    visit(scene->mRootNode, units);
  state.ok = !state.parts.empty();
  return true;
}
} // namespace

void Model::finalizeImport(const Ptr<void> &opaque) {
  auto state = std::static_pointer_cast<AsyncModelState>(opaque);
  ScopedLoadContext context(state->group);
  Hashmap<String, Ptr<Texture>> cache;
  Array<Array<Ptr<Texture>>> images;
  images.resize(state->materials.size());
  for (size_t i = 0; i < state->materials.size(); ++i) {
    auto &material = state->materials[i];
    auto &textures = images[i];
    textures.resize(6);
    for (size_t t = 0; t < 6; ++t) {
      const auto &source = material.textures[t];
      if (!source.present)
        continue;
      if (const auto found = cache.find(source.key); found != cache.end()) {
        textures[t] = found->second;
        continue;
      }
      auto image = Texture::create();
      if (!source.path.empty())
        image->load(source.path, state->options.maxTextureSize);
      else if (source.compressed) {
        auto buffer = makeBuffer(source.bytes.size(), BufferType::Binary);
        std::memcpy(buffer.data(), source.bytes.data(), source.bytes.size());
        image->load(buffer, state->options.maxTextureSize);
      } else
        image->loadFromMemory(const_cast<char *>(source.bytes.data()),
                              source.width, source.height, source.format,
                              source.channels);
      textures[t] = image;
      cache.insert_or_assign(source.key, image);
    }
    if (!textures[2]) {
      auto image = Texture::create();
      image->fill(
          Color(0, std::clamp(material.surface.roughnessFactor, 0.0f, 1.0f),
                std::clamp(material.surface.metallicFactor, 0.0f, 1.0f), 1));
      textures[2] = image;
      material.surface.constantMetallicRoughness = true;
    }
    const auto &emission = material.surface.emissiveFactor;
    if (!textures[4] && (emission.r > 0 || emission.g > 0 || emission.b > 0)) {
      textures[4] = Texture::create();
      textures[4]->fill(Color(1.0f));
    }
  }
  for (auto &part : state->parts) {
    auto child = makePtr<Model>(GlobalAllocator::get());
    child->mesh = std::move(part.mesh);
    child->surface = state->materials[part.material].surface;
    child->textures = images[part.material];
    child->materialCount = 1;
    child->updateBounds();
    const auto data = child->mesh.data();
    auto buffers = createVertexIndexBuffer(data->vertices, data->indices);
    child->vertexBuffer = std::move(buffers.first);
    child->indexBuffer = std::move(buffers.second);
    child->lodIndexBuffers =
        createLodIndexBuffers(child->mesh, child->indexBuffer);
    parts.push_back(std::move(child));
  }
  Array<bool> used(state->materials.size(), false);
  for (const auto &part : state->parts)
    used[part.material] = true;
  materialCount =
      static_cast<UInt32>(std::count(used.begin(), used.end(), true));
  if (parts.empty())
    return;
  localAABB = parts[0]->localAABB;
  for (const auto &part : parts) {
    const auto &b = part->localAABB;
    localAABB.min = Vec3(std::min(localAABB.min.x, b.min.x),
                         std::min(localAABB.min.y, b.min.y),
                         std::min(localAABB.min.z, b.min.z));
    localAABB.max = Vec3(std::max(localAABB.max.x, b.max.x),
                         std::max(localAABB.max.y, b.max.y),
                         std::max(localAABB.max.z, b.max.z));
  }
  if (parts.size() == 1) {
    const auto child = parts.front();
    mesh = std::move(child->mesh);
    surface = std::move(child->surface);
    textures = std::move(child->textures);
    vertexBuffer = std::move(child->vertexBuffer);
    indexBuffer = std::move(child->indexBuffer);
    lodIndexBuffers = std::move(child->lodIndexBuffers);
    parts.clear();
  }
}

Model::Model() : textures(6) {}

void Model::load(StringView path) { load(path, LoadOptions{}); }

void Model::load(StringView path, const LoadOptions &options) {
  const TaskGroup group = LoadContext::current();
  group.add();

  this->vertexBuffer = nullptr;
  this->tangentBuffer = nullptr;
  this->animationVertexBuffer = nullptr;
  this->indexBuffer = nullptr;
  this->lodIndexBuffers.clear();
  this->textures.assign(6, nullptr);
  parts.clear();
  mesh = Mesh();
  surface = {};
  materialCount = 0;

  const String pathStr(path);
  auto state = makePtr<AsyncModelState>();
  state->embeddedTextures.resize(6);
  state->options = options;
  state->group = group;
  state->debugName = "Model::load(" + pathStr + ")";
  const Ptr<void> stateVoid = std::static_pointer_cast<void>(state);
  this->data = stateVoid;

  state->future = globalThreadPool().submit([state, pathStr]() {
    const char *stage = "start";
    try {
      Assimp::Importer importer;
      const unsigned int flags =
          aiProcess_ValidateDataStructure | aiProcess_LimitBoneWeights |
          aiProcess_JoinIdenticalVertices | aiProcess_Triangulate |
          aiProcess_GenSmoothNormals;
      const aiScene *scene = nullptr;
      String modelBytes;
#if defined(SINEN_PLATFORM_EMSCRIPTEN) || defined(EMSCRIPTEN)
      stage = "read file";
      modelBytes = AssetReader::readAsString(pathStr);
      if (!modelBytes.empty()) {
        Log::info("Model path is empty, trying to load from memory");
        const String hint = assimpHintFromPath(pathStr);
        stage = "assimp read memory";
        scene = importer.ReadFileFromMemory(
            modelBytes.data(), modelBytes.size(), flags, hint.c_str());
      }
#else
      stage = "resolve model path";
      if (AssetReader::isArchiveMounted() && AssetReader::exists(pathStr)) {
        stage = "read archive model";
        modelBytes = AssetReader::readAsString(pathStr);
        const String hint = assimpHintFromPath(pathStr);
        stage = "assimp read archive memory";
        scene = importer.ReadFileFromMemory(
            modelBytes.data(), modelBytes.size(), flags, hint.c_str());
      } else {
        stage = "assimp read file";
        auto fullFilePath = AssetReader::getFilePath(pathStr);
        scene = importer.ReadFile(fullFilePath.c_str(), flags);
      }
#endif
      if (!scene) {
        Log::error("Cannot import model {}: {}", pathStr,
                   importer.GetErrorString());
        state->ok = false;
        return;
      }

      stage = "static scene hierarchy and materials";
      if (importStaticScene(scene, *state, pathStr))
        return;

      Mesh mesh;
      AABB aabb;
      Model::BoneMap boneMap;
      SkeletalAnimation skeletalAnimation;

      stage = "load bones";
      loadBone(scene, boneMap);
      stage = "load animation";
      loadAnimation(scene, skeletalAnimation, boneMap);
      stage = "load mesh";
      loadMesh(scene, mesh, aabb);
      stage = "calculate tangents";
      calcTangents(scene, mesh);
      stage = "optimize mesh";
      optimizeMesh(mesh);

      stage = "extract embedded textures";
      Array<EmbeddedTextureData> embedded;
      embedded.resize(6);
      for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
        if (!embedded[0].present) {
          embedded[0] = extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                               scene->mMaterials[i],
                                               aiTextureType_BASE_COLOR);
        }
        if (!embedded[1].present) {
          embedded[1] = extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                               scene->mMaterials[i],
                                               aiTextureType_NORMALS);
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
    } catch (const std::exception &e) {
      state->ok = false;
      Log::error("Model async load failed at {} ({}): {}", stage,
                 pathStr.c_str(), e.what());
    } catch (...) {
      state->ok = false;
      Log::error("Model async load failed at {} ({}) with an unknown exception",
                 stage, pathStr.c_str());
    }
  });

  scheduleFuturePoll(
      state, group,
      [](std::function<void()> f) { Graphics::addPreDrawFunc(std::move(f)); },
      [this, state, stateVoid, group] {
        if (state->finalized.exchange(true)) {
          return;
        }

        if (this->data != stateVoid) {
          return;
        }

        if (!state->ok) {
          this->data.reset();
          return;
        }

        if (state->staticScene) {
          finalizeImport(stateVoid);
          this->data.reset();
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
              texture->loadFromMemory(const_cast<char *>(d.bytes.data()),
                                      d.width, d.height, d.format, d.channels);
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
        this->lodIndexBuffers =
            createLodIndexBuffers(this->mesh, this->indexBuffer);

        this->data.reset();
      },
      [this, stateVoid] {
        if (this->data == stateVoid) {
          this->data.reset();
        }
      });
}
void Model::load(const Buffer &buffer) { load(buffer, ""); }

void Model::load(const Buffer &buffer, StringView formatHint) {
  load(buffer, formatHint, LoadOptions{});
}

void Model::load(const Buffer &buffer, StringView formatHint,
                 const LoadOptions &options) {
  const TaskGroup group = LoadContext::current();
  group.add();

  this->vertexBuffer = nullptr;
  this->tangentBuffer = nullptr;
  this->animationVertexBuffer = nullptr;
  this->indexBuffer = nullptr;
  this->lodIndexBuffers.clear();
  this->textures.assign(6, nullptr);
  parts.clear();
  mesh = Mesh();
  surface = {};
  materialCount = 0;

  auto state = makePtr<AsyncModelState>();
  state->options = options;
  state->group = group;
  state->embeddedTextures.resize(6);
  state->debugName = "Model::load(buffer, " + String(formatHint) + ")";
  const Ptr<void> stateVoid = std::static_pointer_cast<void>(state);
  this->data = stateVoid;

  const Buffer buf = buffer;
  const String hint(formatHint);
  state->future = globalThreadPool().submit([state, buf, hint]() {
    const char *stage = "start";
    try {
      Assimp::Importer importer;
      stage = "assimp read memory";
      const aiScene *scene = importer.ReadFileFromMemory(
          buf.data(), buf.size(),
          aiProcess_ValidateDataStructure | aiProcess_LimitBoneWeights |
              aiProcess_JoinIdenticalVertices | aiProcess_Triangulate |
              aiProcess_GenSmoothNormals,
          hint.empty() ? nullptr : hint.c_str());
      if (!scene) {
        state->ok = false;
        return;
      }

      if (importStaticScene(scene, *state, ""))
        return;

      Mesh mesh;
      AABB aabb;
      Model::BoneMap boneMap;
      SkeletalAnimation skeletalAnimation;

      stage = "load bones";
      loadBone(scene, boneMap);
      stage = "load animation";
      loadAnimation(scene, skeletalAnimation, boneMap);
      stage = "load mesh";
      loadMesh(scene, mesh, aabb);
      stage = "calculate tangents";
      calcTangents(scene, mesh);
      stage = "optimize mesh";
      optimizeMesh(mesh);

      stage = "extract embedded textures";
      Array<EmbeddedTextureData> embedded;
      embedded.resize(6);
      for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
        if (!embedded[0].present) {
          embedded[0] = extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                               scene->mMaterials[i],
                                               aiTextureType_BASE_COLOR);
        }
        if (!embedded[1].present) {
          embedded[1] = extractEmbeddedTexture(const_cast<aiScene *>(scene),
                                               scene->mMaterials[i],
                                               aiTextureType_NORMALS);
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
    } catch (const std::exception &e) {
      state->ok = false;
      Log::error("Model async load(buffer) failed at {}: {}", stage, e.what());
    } catch (...) {
      state->ok = false;
      Log::error("Model async load(buffer) failed at {} with an unknown "
                 "exception",
                 stage);
    }
  });

  scheduleFuturePoll(
      state, group,
      [](std::function<void()> f) { Graphics::addPreDrawFunc(std::move(f)); },
      [this, state, stateVoid, group] {
        if (state->finalized.exchange(true)) {
          return;
        }

        if (this->data != stateVoid) {
          return;
        }

        if (!state->ok) {
          this->data.reset();
          return;
        }

        if (state->staticScene) {
          finalizeImport(stateVoid);
          this->data.reset();
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
              texture->loadFromMemory(const_cast<char *>(d.bytes.data()),
                                      d.width, d.height, d.format, d.channels);
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
        this->lodIndexBuffers =
            createLodIndexBuffers(this->mesh, this->indexBuffer);

        this->data.reset();
      },
      [this, stateVoid] {
        if (this->data == stateVoid) {
          this->data.reset();
        }
      });
}

void Model::loadFromVertexArray(const Mesh &m, bool optimize) {
  data.reset();
  parts.clear();
  surface = {};
  materialCount = 0;
  this->mesh = Mesh(*m.data());
  if (optimize) {
    optimizeMesh(this->mesh);
  }
  updateBounds();
  auto mesh = this->mesh.data();
  auto viBuffer = createVertexIndexBuffer(mesh->vertices, mesh->indices);
  this->vertexBuffer = viBuffer.first;
  this->indexBuffer = viBuffer.second;
  this->lodIndexBuffers = createLodIndexBuffers(this->mesh, this->indexBuffer);
}

bool Model::loadTransientFromVertexArray(
    const Mesh &source, const Ptr<gpu::CommandBuffer> &commands) {
  const auto data = source.data();
  if (!commands || !data || data->vertices.empty() || data->indices.empty()) {
    return false;
  }
  if (data->vertices.size() > UINT32_MAX / sizeof(Vertex) ||
      data->indices.size() > UINT32_MAX / sizeof(UInt32)) {
    return false;
  }
  gpu::Buffer::CreateInfo vertexInfo{};
  vertexInfo.allocator = commands->getCreateInfo().allocator;
  vertexInfo.size = data->vertices.size() * sizeof(Vertex);
  vertexInfo.usage = gpu::BufferUsage::Vertex;
  auto vertices =
      commands->createTransientBuffer(vertexInfo, data->vertices.data());
  if (!vertices) {
    return false;
  }
  gpu::Buffer::CreateInfo indexInfo{};
  indexInfo.allocator = vertexInfo.allocator;
  indexInfo.size = data->indices.size() * sizeof(UInt32);
  indexInfo.usage = gpu::BufferUsage::Index;
  auto indices =
      commands->createTransientBuffer(indexInfo, data->indices.data());
  if (!indices) {
    return false;
  }
  mesh = source;
  this->data.reset();
  parts.clear();
  surface = {};
  materialCount = 0;
  vertexBuffer = std::move(vertices);
  indexBuffer = std::move(indices);
  lodIndexBuffers.clear();
  updateBounds();
  return true;
}

void Model::updateBounds() {
  auto mesh = this->mesh.data();
  AABB aabb;
  if (!mesh->vertices.empty())
    aabb.min = aabb.max = mesh->vertices.front().position;
  for (auto &v : mesh->vertices) {
    aabb.min.x = Math::min(aabb.min.x, v.position.x);
    aabb.min.y = Math::min(aabb.min.y, v.position.y);
    aabb.min.z = Math::min(aabb.min.z, v.position.z);
    aabb.max.x = Math::max(aabb.max.x, v.position.x);
    aabb.max.y = Math::max(aabb.max.y, v.position.y);
    aabb.max.z = Math::max(aabb.max.z, v.position.z);
  }
  this->localAABB = aabb;
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

UInt32 Model::getLodCount() const {
  auto data = mesh.data();
  return data->lods.empty() ? 1u : static_cast<UInt32>(data->lods.size());
}

UInt32 Model::selectLod(float distance) const {
  auto data = mesh.data();
  if (data->lods.empty()) {
    return 0;
  }

  const Vec3 extent = localAABB.max - localAABB.min;
  const float radius = Math::max(extent.length() * 0.5f, 0.001f);
  const float normalizedDistance = distance / radius;

  UInt32 selected = 0;
  for (UInt32 i = 1; i < data->lods.size(); ++i) {
    if (normalizedDistance >= data->lods[i].threshold) {
      selected = i;
    }
  }
  return selected;
}

const Array<UInt32> &Model::getIndicesForLod(UInt32 lod) const {
  auto data = mesh.data();
  if (lod < data->lods.size() && !data->lods[lod].indices.empty()) {
    return data->lods[lod].indices;
  }
  return data->indices;
}

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

Array<Ptr<gpu::Buffer>> createLodIndexBuffers(const Mesh &mesh,
                                              const Ptr<gpu::Buffer> &base) {
  Array<Ptr<gpu::Buffer>> buffers;
  auto data = mesh.data();
  buffers.reserve(data->lods.size());
  for (size_t i = 0; i < data->lods.size(); ++i) {
    auto &lod = data->lods[i];
    if (i == 0) {
      buffers.push_back(base);
      continue;
    }
    if (lod.indices.empty()) {
      buffers.push_back(nullptr);
      continue;
    }
    buffers.push_back(createBuffer(lod.indices.size() * sizeof(UInt32),
                                   lod.indices.data(),
                                   gpu::BufferUsage::Index));
  }
  return buffers;
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

  if (!data || size == 0)
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
  constexpr size_t shaderBoneCapacity = 256;
  const auto matrixCount = boneMatrices.size() < shaderBoneCapacity
                               ? boneMatrices.size()
                               : shaderBoneCapacity;
  auto size = shaderBoneCapacity * sizeof(Mat4);
  auto *ptr = (Mat4 *)GlobalAllocator::get()->allocate(size);
  for (size_t i = 0; i < shaderBoneCapacity; ++i) {
    ptr[i] = Mat4::identity();
  }
  if (matrixCount > 0) {
    memcpy(ptr, boneMatrices.data(), matrixCount * sizeof(Mat4));
  }
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
  return getTexture(type) != nullptr;
}
Ptr<Texture> Model::getTexture(TextureKey type) const {
  const auto index = static_cast<UInt32>(type);
  if (index >= textures.size())
    return nullptr;
  if (textures[index])
    return textures[index];
  for (const auto &part : parts)
    if (auto t = part->getTexture(type))
      return t;
  return nullptr;
}
void Model::setTexture(TextureKey type, const Ptr<Texture> &texture) {
  const auto index = static_cast<UInt32>(type);
  if (index >= textures.size())
    return;
  textures[index] = texture;
  for (const auto &part : parts)
    part->setTexture(type, texture);
}

Mesh Model::getMesh() const {
  if (!parts.empty() && mesh.data()->vertices.empty()) {
    auto target = mesh.data();
    for (const auto &part : parts) {
      const auto source = part->getMesh().data();
      const auto offset = static_cast<UInt32>(target->vertices.size());
      target->vertices.insert(target->vertices.end(), source->vertices.begin(),
                              source->vertices.end());
      for (auto index : source->indices)
        target->indices.push_back(offset + index);
    }
  }
  return mesh;
}

UInt32 Model::getTriangleCount() const {
  UInt32 count = 0;
  if (parts.empty())
    return static_cast<UInt32>(mesh.data()->indices.size() / 3);
  for (const auto &part : parts)
    count += part->getTriangleCount();
  return count;
}

bool Model::isReady() const {
  if (data)
    return false;
  if (parts.empty() && (!vertexBuffer || !indexBuffer))
    return false;
  for (const auto &part : parts)
    if (!part->isReady())
      return false;
  for (const auto &texture : textures)
    if (texture && !texture->getRaw())
      return false;
  return true;
}

} // namespace sinen
