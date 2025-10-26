// std
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

// internal
#include "../../graphics/graphics_system.hpp"
#include "assimp/matrix4x4.h"
#include "assimp/postprocess.h"
#include "glm/ext/quaternion_common.hpp"
#include "glm/fwd.hpp"
#include "model_data.hpp"
#include <asset/asset.hpp>
#include <core/core.hpp>
#include <graphics/graphics.hpp>
#include <math/math.hpp>

#include <assimp/DefaultLogger.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/matrix_common.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_common.hpp"
#include "libs/assimp/code/AssetLib/3MF/3MFXmlTags.h"
#include "sol/types.hpp"

#include <glm/ext/vector_common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <ranges>

namespace sinen {
enum class LoadState { Version, Vertex, Indices };
std::vector<glm::vec3> getVector3sFromKey(const aiVectorKey *keys,
                                          uint32_t count) {
  std::vector<glm::vec3> result;
  for (uint32_t i = 0; i < count; ++i) {
    result.push_back(
        glm::vec3(keys[i].mValue.x, keys[i].mValue.y, keys[i].mValue.z));
  }
  return result;
};
std::vector<float> getTimesFromVector3Key(const aiVectorKey *keys,
                                          uint32_t count) {
  std::vector<float> result;
  for (uint32_t i = 0; i < count; ++i) {
    result.push_back(static_cast<float>(keys[i].mTime));
  }
  return result;
};
std::vector<glm::quat> getRotatesFromKey(const aiQuatKey *keys,
                                         uint32_t count) {
  std::vector<glm::quat> result;
  for (uint32_t i = 0; i < count; ++i) {
    result.push_back(glm::quat(keys[i].mValue.w, keys[i].mValue.x,
                               keys[i].mValue.y, keys[i].mValue.z));
  }
  return result;
};
std::vector<float> getTimesFromQuatKey(const aiQuatKey *keys, uint32_t count) {
  std::vector<float> result;
  for (uint32_t i = 0; i < count; ++i) {
    result.push_back(static_cast<float>(keys[i].mTime));
  }
  return result;
};
glm::mat4 convertMatrix(const aiMatrix4x4 &m) {
  glm::mat4 mat = glm::make_mat4(&m.a1);
  mat = glm::transpose(mat); // Assimp is column major
  return mat;
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
Model::Model() {}

void loadAnimation(const aiScene *scene, SkeletalAnimation &skeletalAnimation,
                   const Model::BoneMap &boneMap) {
  if (scene->HasAnimations()) {
    {

      auto anim = scene->mAnimations[0];
      std::unordered_map<std::string, NodeAnimation> nodeAnimMap;
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
      std::vector<uint32_t> ids;
      std::vector<float> weights;
    };
    for (int i = 0; i < scene->mNumMeshes; i++) {
      const aiMesh *aimesh = scene->mMeshes[i];
      std::unordered_map<uint32_t, BoneData> boneData;

      for (uint32_t j = 0; j < aimesh->mNumBones; j++) {
        aiBone *bone = aimesh->mBones[j];
        std::string boneName = bone->mName.C_Str();

        uint32_t index = boneMap.at(boneName).index;

        for (uint32_t k = 0; k < bone->mNumWeights; ++k) {
          uint32_t vertexId = bone->mWeights[k].mVertexId;
          float weight = bone->mWeights[k].mWeight;
          boneData[vertexId].ids.push_back(index);
          boneData[vertexId].weights.push_back(weight);
        }
      }

      aiColor3D acolor(1, 1, 1);
      if (scene->HasMaterials())
        scene->mMaterials[aimesh->mMaterialIndex]->Get(AI_MATKEY_BASE_COLOR,
                                                       acolor);
      for (uint32_t j = 0; j < aimesh->mNumVertices; ++j) {
        AnimationVertex animationVertex;
        const auto &ids = boneData[j].ids;
        const auto &ws = boneData[j].weights;
        float temp = 0.f;
        assert(ids.size() <= 4);
        assert(ws.size() <= 4);
        for (int k = 0; k < 4; ++k) {
          animationVertex.boneIDs[k] = (k < ids.size()) ? float(ids[k]) : 0.0f;
          animationVertex.boneWeights[k] = (k < ws.size()) ? ws[k] : 0.0f;
        }
        skeletalAnimation.animationVertices.push_back(animationVertex);
      }
    }
  }
}

void loadBone(const aiScene *scene, Model::BoneMap &boneMap) {
  for (int i = 0; i < scene->mNumMeshes; i++) {
    const aiMesh *aimesh = scene->mMeshes[i];
    for (uint32_t j = 0; j < aimesh->mNumBones; j++) {
      aiBone *bone = aimesh->mBones[j];
      std::string boneName = bone->mName.C_Str();

      if (!boneMap.contains(boneName)) {
        BoneInfo boneInfo;
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
      const aiMesh *aimesh = scene->mMeshes[i];
      // Process the mesh data
      for (uint32_t j = 0; j < aimesh->mNumVertices; j++) {
        const aiVector3D &pos = aimesh->mVertices[j];
        const aiVector3D &norm = aimesh->mNormals[j];
        glm::vec2 uv(0.f, 0.f);
        if (aimesh->HasTextureCoords(0)) {
          const aiVector3D &aiuv = aimesh->mTextureCoords[0][j];
          uv.x = aiuv.x;
          uv.y = aiuv.y;
        }
        Color color = Color(1.f, 1.f, 1.f, 1.f);
        if (aimesh->HasVertexColors(j)) {
          const aiColor4D &c = aimesh->mColors[0][j];
          color = Color(c.r, c.g, c.b, c.a);
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

        mesh.vertices.push_back(v);
      }
      // Process the indices
      uint32_t maxIndex = 0;
      for (uint32_t j = 0; j < aimesh->mNumFaces; j++) {
        const aiFace &face = aimesh->mFaces[j];
        for (uint32_t k = 0; k < face.mNumIndices; k++) {
          uint32_t index = face.mIndices[k];
          maxIndex = Math::max(maxIndex, index);
          mesh.indices.push_back(baseIndex + index);
        }
      }
      baseIndex += aimesh->mNumVertices;
    }
  }
}

void calcTangents(const aiScene *scene, Mesh &mesh) {
  auto vertexCount = mesh.vertices.size();
  mesh.tangents.resize(vertexCount, glm::vec4(0.0f));

  std::vector tangentAccum(vertexCount, glm::vec3(0.0f));
  std::vector handedness(vertexCount, 1.0f);

  for (size_t i = 0; i < mesh.indices.size(); i += 3) {
    Vertex &v0 = mesh.vertices[mesh.indices[i]];
    Vertex &v1 = mesh.vertices[mesh.indices[i + 1]];
    Vertex &v2 = mesh.vertices[mesh.indices[i + 2]];

    glm::vec3 edge1 = v1.position - v0.position;
    glm::vec3 edge2 = v2.position - v0.position;
    glm::vec2 deltaUV1 = v1.uv - v0.uv;
    glm::vec2 deltaUV2 = v2.uv - v0.uv;

    float det = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
    if (fabs(det) < 1e-6f)
      continue;
    float f = 1.0f / det;

    glm::vec3 tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * f;
    glm::vec3 bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * f;

    tangentAccum[mesh.indices[i]] += tangent;
    tangentAccum[mesh.indices[i + 1]] += tangent;
    tangentAccum[mesh.indices[i + 2]] += tangent;

    // optional: handedness check
    glm::vec3 N = glm::normalize(v0.normal);
    float sign =
        (glm::dot(glm::cross(N, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;
    handedness[mesh.indices[i]] = sign;
    handedness[mesh.indices[i + 1]] = sign;
    handedness[mesh.indices[i + 2]] = sign;
  }

  for (size_t j = 0; j < mesh.vertices.size(); ++j) {
    glm::vec3 T = glm::normalize(tangentAccum[j]);
    mesh.tangents[j] = glm::vec4(T, handedness[j]);
  }
}

std::optional<Texture> loadTexture(aiScene *scene, aiMaterial *material,
                                   aiTextureType type) {
  aiString texPath;
  if (material->GetTexture(type, 0, &texPath) != AI_SUCCESS) {
    return std::nullopt;
  }
  auto *aiTex = scene->GetEmbeddedTexture(texPath.C_Str());
  if (!aiTex) {
    return std::nullopt;
  }
  Texture texture;
  std::vector<char> buffer;
  if (aiTex->mHeight == 0) {
    buffer.resize(aiTex->mWidth);
    memcpy(buffer.data(), aiTex->pcData, aiTex->mWidth);
    texture.loadFromMemory(buffer);
  } else {
    texture.loadFromMemory(aiTex->pcData, aiTex->mWidth, aiTex->mHeight);
  }
  return texture;
}
void loadMaterial(aiScene *scene, Material &material) {
  for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
    {
      auto texture =
          loadTexture(scene, scene->mMaterials[i], aiTextureType_BASE_COLOR);
      if (texture.has_value()) {
        material.appendTexture(texture.value());
      }
    }
    {
      auto texture =
          loadTexture(scene, scene->mMaterials[i], aiTextureType_NORMALS);
      if (texture.has_value()) {
        material.appendTexture(texture.value());
      }
    }
    {
      auto texture = loadTexture(scene, scene->mMaterials[i],
                                 aiTextureType_DIFFUSE_ROUGHNESS);
      if (texture.has_value()) {
        material.appendTexture(texture.value());
      }
    }
    {
      auto texture =
          loadTexture(scene, scene->mMaterials[i], aiTextureType_METALNESS);
      if (texture.has_value()) {
        material.appendTexture(texture.value());
      }
    }
    {
      auto texture =
          loadTexture(scene, scene->mMaterials[i], aiTextureType_EMISSIVE);
      if (texture.has_value()) {
        material.appendTexture(texture.value());
      }
    }
    {
      auto texture =
          loadTexture(scene, scene->mMaterials[i], aiTextureType_LIGHTMAP);
      if (texture.has_value()) {
        material.appendTexture(texture.value());
      }
    }
  }
  if (material.getTextureCount() == 0) {
    Texture defaultTexture;
    defaultTexture.fill(Color(1.f));
    material.appendTexture(defaultTexture);
  }
}

void Model::load(std::string_view path) {

  auto fullFilePath = AssetIO::convertFilePath(AssetType::Model, path.data());
  // Assimp
  Assimp::Importer importer;
  const auto *scene = importer.ReadFile(
      fullFilePath.c_str(),
      aiProcess_ValidateDataStructure | aiProcess_LimitBoneWeights |
          aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);
  if (!scene) {
    Logger::error("Error loading model: %s", importer.GetErrorString());
    return;
  }
  skeletalAnimation.owner = this;
  loadBone(scene, this->boneMap);
  loadAnimation(scene, this->skeletalAnimation, this->boneMap);
  loadMesh(scene, this->mesh, this->localAABB);
  loadMaterial(const_cast<aiScene *>(scene), this->material);
  calcTangents(scene, this->mesh);

  mesh.indexCount = mesh.indices.size();
  auto viBuffer = createVertexIndexBuffer(mesh.vertices, mesh.indices);
  this->vertexBuffer =
      createBuffer(mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(),
                   px::BufferUsage::Vertex);
  this->animationVertexBuffer =
      createAnimationVertexBuffer(skeletalAnimation.animationVertices);
  this->tangentBuffer =
      createBuffer(mesh.tangents.size() * sizeof(glm::vec4),
                   mesh.tangents.data(), px::BufferUsage::Vertex);
  this->indexBuffer = createBuffer(mesh.indices.size() * sizeof(uint32_t),
                                   mesh.indices.data(), px::BufferUsage::Index);
}

void Model::loadFromVertexArray(const Mesh &mesh) {
  this->mesh = mesh;
  AABB aabb;
  for (auto &v : mesh.vertices) {
    aabb.min.x = Math::min(aabb.min.x, v.position.x);
    aabb.min.y = Math::min(aabb.min.y, v.position.y);
    aabb.min.z = Math::min(aabb.min.z, v.position.z);
    aabb.max.x = Math::max(aabb.max.x, v.position.x);
    aabb.max.y = Math::max(aabb.max.y, v.position.y);
    aabb.max.z = Math::max(aabb.max.z, v.position.z);
  }
  this->localAABB = aabb;
  auto viBuffer = createVertexIndexBuffer(mesh.vertices, mesh.indices);
  this->vertexBuffer = viBuffer.first;
  this->indexBuffer = viBuffer.second;
}

void Model::loadSprite() { *this = GraphicsSystem::sprite; }
void Model::loadBox() { *this = GraphicsSystem::box; }

const AABB &Model::getAABB() const { return this->localAABB; }

std::pair<px::Ptr<px::Buffer>, px::Ptr<px::Buffer>>
createVertexIndexBuffer(const std::vector<Vertex> &vertices,
                        const std::vector<uint32_t> &indices) {
  auto allocator = GraphicsSystem::getAllocator();
  auto device = GraphicsSystem::getDevice();
  size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
  px::Ptr<px::Buffer> vertexBuffer, indexBuffer;
  px::Buffer::CreateInfo vertexBufferInfo{};
  vertexBufferInfo.allocator = allocator;
  vertexBufferInfo.size = vertexBufferSize;
  vertexBufferInfo.usage = px::BufferUsage::Vertex;
  vertexBuffer = device->CreateBuffer(vertexBufferInfo);

  px::Buffer::CreateInfo indexBufferInfo{};
  indexBufferInfo.allocator = allocator;
  indexBufferInfo.size = indices.size() * sizeof(uint32_t);
  indexBufferInfo.usage = px::BufferUsage::Index;
  indexBuffer = device->CreateBuffer(indexBufferInfo);

  px::Ptr<px::TransferBuffer> transferBuffer;
  {
    {
      px::TransferBuffer::CreateInfo info{};
      info.allocator = allocator;
      info.size = vertexBufferSize;
      info.usage = px::TransferBufferUsage::Upload;
      transferBuffer = device->CreateTransferBuffer(info);
      auto *pMapped = transferBuffer->Map(false);
      memcpy(pMapped, vertices.data(), vertices.size() * sizeof(Vertex));
      transferBuffer->Unmap();
    }
    {
      px::CommandBuffer::CreateInfo info{};
      info.allocator = allocator;
      auto commandBuffer = device->AcquireCommandBuffer(info);
      {

        auto copyPass = commandBuffer->BeginCopyPass();
        {

          px::BufferTransferInfo src{};
          src.offset = 0;
          src.transferBuffer = transferBuffer;
          px::BufferRegion dst{};
          dst.offset = 0;
          dst.size = vertexBufferSize;
          dst.buffer = vertexBuffer;
          copyPass->UploadBuffer(src, dst, false);
        }
        commandBuffer->EndCopyPass(copyPass);
      }
      device->SubmitCommandBuffer(commandBuffer);
    }
  }
  {
    px::TransferBuffer::CreateInfo info{};
    info.allocator = allocator;
    info.size = indexBufferInfo.size;
    info.usage = px::TransferBufferUsage::Upload;
    transferBuffer = device->CreateTransferBuffer(info);
    auto *pMapped = transferBuffer->Map(false);
    memcpy(pMapped, indices.data(), indexBufferInfo.size);
    transferBuffer->Unmap();
  }
  {
    {
      px::CommandBuffer::CreateInfo info{};
      info.allocator = allocator;
      auto commandBuffer = device->AcquireCommandBuffer(info);
      {

        auto copyPass = commandBuffer->BeginCopyPass();
        {

          px::BufferTransferInfo src{};
          src.offset = 0;
          src.transferBuffer = transferBuffer;
          px::BufferRegion dst{};
          dst.offset = 0;
          dst.size = indexBufferInfo.size;
          dst.buffer = indexBuffer;
          copyPass->UploadBuffer(src, dst, false);
        }
        commandBuffer->EndCopyPass(copyPass);
      }
      device->SubmitCommandBuffer(commandBuffer);
    }
  }
  return std::make_pair(vertexBuffer, indexBuffer);
}
px::Ptr<px::Buffer>
createAnimationVertexBuffer(const std::vector<AnimationVertex> &vertices) {
  if (vertices.empty())
    return nullptr;
  auto allocator = GraphicsSystem::getAllocator();
  auto device = GraphicsSystem::getDevice();
  size_t vertexBufferSize = vertices.size() * sizeof(AnimationVertex);
  px::Ptr<px::Buffer> vertexBuffer;
  px::Buffer::CreateInfo vertexBufferInfo{};
  vertexBufferInfo.allocator = allocator;
  vertexBufferInfo.size = vertexBufferSize;
  vertexBufferInfo.usage = px::BufferUsage::Vertex;
  vertexBuffer = device->CreateBuffer(vertexBufferInfo);

  px::Ptr<px::TransferBuffer> transferBuffer;
  {
    {
      px::TransferBuffer::CreateInfo info{};
      info.allocator = allocator;
      info.size = vertexBufferSize;
      info.usage = px::TransferBufferUsage::Upload;
      transferBuffer = device->CreateTransferBuffer(info);
      auto *pMapped = transferBuffer->Map(false);
      memcpy(pMapped, vertices.data(),
             vertices.size() * sizeof(AnimationVertex));
      transferBuffer->Unmap();
    }
    {
      px::CommandBuffer::CreateInfo info{};
      info.allocator = allocator;
      auto commandBuffer = device->AcquireCommandBuffer(info);
      {

        auto copyPass = commandBuffer->BeginCopyPass();
        {

          px::BufferTransferInfo src{};
          src.offset = 0;
          src.transferBuffer = transferBuffer;
          px::BufferRegion dst{};
          dst.offset = 0;
          dst.size = vertexBufferSize;
          dst.buffer = vertexBuffer;
          copyPass->UploadBuffer(src, dst, false);
        }
        commandBuffer->EndCopyPass(copyPass);
      }
      device->SubmitCommandBuffer(commandBuffer);
    }
  }
  return vertexBuffer;
}
px::Ptr<px::Buffer> createBuffer(size_t size, void *data,
                                 px::BufferUsage usage) {

  if (!data)
    return nullptr;
  auto allocator = GraphicsSystem::getAllocator();
  auto device = GraphicsSystem::getDevice();
  px::Ptr<px::Buffer> buffer;
  px::Buffer::CreateInfo vertexBufferInfo{};
  vertexBufferInfo.allocator = allocator;
  vertexBufferInfo.size = size;
  vertexBufferInfo.usage = usage;
  buffer = device->CreateBuffer(vertexBufferInfo);

  px::Ptr<px::TransferBuffer> transferBuffer;
  {
    {
      px::TransferBuffer::CreateInfo info{};
      info.allocator = allocator;
      info.size = size;
      info.usage = px::TransferBufferUsage::Upload;
      transferBuffer = device->CreateTransferBuffer(info);
      auto *pMapped = transferBuffer->Map(false);
      memcpy(pMapped, data, size);
      transferBuffer->Unmap();
    }
    {
      px::CommandBuffer::CreateInfo info{};
      info.allocator = allocator;
      auto commandBuffer = device->AcquireCommandBuffer(info);
      {

        auto copyPass = commandBuffer->BeginCopyPass();
        {

          px::BufferTransferInfo src{};
          src.offset = 0;
          src.transferBuffer = transferBuffer;
          px::BufferRegion dst{};
          dst.offset = 0;
          dst.size = size;
          dst.buffer = buffer;
          copyPass->UploadBuffer(src, dst, false);
        }
        commandBuffer->EndCopyPass(copyPass);
      }
      device->SubmitCommandBuffer(commandBuffer);
    }
  }
  return buffer;
}

UniformData Model::getBoneUniformData() const { return this->boneUniformData; }
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
  boneUniformData.clear();

  std::vector<glm::mat4> boneMatrices(matrices.size());
  int index = 0;
  for (auto &m : matrices) {
    boneMatrices[index] = glm::transpose(m);
    index++;
  }
  boneUniformData.addMatrices(boneMatrices);
}
void SkeletalAnimation::load(
    const Node &root, float ticksPerSecond, float duration,
    std::unordered_map<std::string, NodeAnimation> &nodeAnimationMap) {
  this->root = root;
  this->ticksPerSecond = ticksPerSecond;
  this->duration = duration;
  this->globalInverseTransform = glm::inverse(root.transformation);
  this->nodeAnimMap = nodeAnimationMap;
}
void SkeletalAnimation::update(float timeInSeconds) {
  float ticksPerSecond =
      this->ticksPerSecond != 0 ? this->ticksPerSecond : 25.0f;
  float timeInTicks = timeInSeconds * ticksPerSecond;
  float animationTime = Math::fmod(timeInTicks, duration);

  readNodeHierarchy(animationTime, root, glm::mat4(1.0f));
}
void SkeletalAnimation::readNodeHierarchy(float animTime, const Node &node,
                                          const glm::mat4 &parentTransform) {
  std::string nodeName = node.name;

  auto nodeTransform = node.transformation;
  if (nodeAnimMap.contains(nodeName)) {
    nodeTransform = interpolateTransform(nodeAnimMap[nodeName], animTime);
  }

  auto globalTransform = parentTransform * nodeTransform;

  const auto &boneMap = owner->getBoneMap();
  if (boneMap.contains(nodeName)) {
    finalBoneMatrices[nodeName] = globalInverseTransform * globalTransform *
                                  boneMap.at(nodeName).offsetMatrix;
  }

  for (const auto &child : node.children) {
    readNodeHierarchy(animTime, child, globalTransform);
  }
}

uint32_t getIndex(float time, const std::vector<float> &keys) {
  auto count = keys.size();
  for (uint32_t i = 0; i < count - 1; ++i) {
    if (time < keys[i + 1]) {
      return i;
    }
  }
  return count - 1;
}

glm::vec3 interpolateVector3(const std::vector<glm::vec3> &keys,
                             const std::vector<float> &keyTimes, float time,
                             bool isScaling = false) {
  auto count = keys.size();
  if (count == 0) {
    if (isScaling) {
      return glm::vec3(1, 1, 1);
    }
    return glm::vec3(0, 0, 0);
  }
  if (count == 1) {
    return keys[0];
  }

  uint32_t i = getIndex(time, keyTimes);
  float t = float((time - keyTimes[i]) / (keyTimes[i + 1] - keyTimes[i]));
  auto k0 = keys[i];
  auto k1 = keys[i + 1];
  return glm::mix(k0, k1, t);
};
glm::quat interpolateQuat(const std::vector<glm::quat> &keys,
                          const std::vector<float> &keyTimes, float time) {
  auto count = keys.size();
  if (count == 0) {
    return glm::quat(1, 0, 0, 0);
  }
  if (count == 1) {
    return keys[0];
  }

  uint32_t i = getIndex(time, keyTimes);
  float t = float((time - keyTimes[i]) / (keyTimes[i + 1] - keyTimes[i]));
  auto k0 = keys[i];
  auto k1 = keys[i + 1];
  auto q = glm::slerp(k0, k1, t);
  q = glm::normalize(q);
  return q;
};

glm::mat4 SkeletalAnimation::interpolateTransform(const NodeAnimation &channel,
                                                  float time) {

  auto scaling =
      interpolateVector3(channel.scale, channel.scaleTime, time, true);
  auto rotation = interpolateQuat(channel.rotation, channel.rotationTime, time);
  auto translation =
      interpolateVector3(channel.position, channel.positionTime, time);

  auto t = glm::translate(glm::mat4(1.0f), translation);
  auto r = glm::toMat4(rotation);
  auto s = glm::scale(glm::mat4(1.0f), scaling);

  auto m = t * r * s;
  return m;
}
std::vector<glm::mat4> SkeletalAnimation::getFinalBoneMatrices() const {
  std::vector<glm::mat4> result(finalBoneMatrices.size(), glm::mat4(1.0f));
  for (const auto &[name, mat] : finalBoneMatrices) {
    if (finalBoneMatrices.contains(name)) {
      result[owner->getBoneMap().at(name).index] = mat;
    } else {
      std::cerr << "[Warning] bone '" << name
                << "' has no finalTransform. Using identity.\n";
    }
  }
  return result;
}
} // namespace sinen