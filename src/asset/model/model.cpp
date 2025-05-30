// std
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

// internal
#include "../../render/px_renderer.hpp"
#include "../../render/render_system.hpp"
#include "assimp/matrix4x4.h"
#include "assimp/postprocess.h"
#include "glm/ext/quaternion_common.hpp"
#include "glm/fwd.hpp"
#include "model_data.hpp"
#include <asset/asset.hpp>
#include <core/core.hpp>
#include <math/math.hpp>
#include <render/renderer.hpp>

#include <assimp/DefaultLogger.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/matrix_common.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_common.hpp"
#include <glm/ext/vector_common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace sinen {
enum class load_state { version, vertex, indices };
Model::Model() { data = std::make_shared<ModelData>(); };
glm::mat4 ConvertMatrix(const aiMatrix4x4 &m) {
  glm::mat4 mat = glm::make_mat4(&m.a1);
  mat = glm::transpose(mat); // Assimp is column major
  return mat;
}

void Model::load(std::string_view str) {
  auto modelData = GetModelData(this->data);
  auto &local_aabb = modelData->local_aabb;
  auto &v_array = modelData->v_array;
  std::stringstream data;
  data << DataStream::open_as_string(AssetType::Model, str);
  std::string str_name = str.data();

  auto fileName = DataStream::convert_file_path(AssetType::Model, str_name);
  // Assimp
  auto &importer = modelData->importer;
  modelData->scene =
      importer.ReadFile(fileName.c_str(), aiProcess_ValidateDataStructure |
                                              aiProcess_LimitBoneWeights |
                                              aiProcess_JoinIdenticalVertices |
                                              aiProcess_Triangulate);
  auto &scene = modelData->scene;
  if (!scene) {
    std::cerr << "Error loading model: " << importer.GetErrorString()
              << std::endl;
    return;
  }
  if (scene->HasAnimations()) {
    modelData->skeletalAnimation.Load(modelData->scene);
    uint32_t vertexOffset = 0;
    struct BoneData {
      std::vector<uint32_t> ids;
      std::vector<float> weights;
      Color color;
    };
    auto &boneMap = modelData->skeletalAnimation.boneMap;
    for (int i = 0; i < scene->mNumMeshes; i++) {
      const aiMesh *mesh = scene->mMeshes[i];
      std::unordered_map<uint32_t, BoneData> boneData;

      for (uint32_t j = 0; j < mesh->mNumBones; j++) {
        aiBone *bone = mesh->mBones[j];
        std::string boneName = bone->mName.C_Str();

        if (!boneMap.contains(boneName)) {
          boneMap[boneName].index = static_cast<uint32_t>(boneMap.size());
          boneMap[boneName].offsetMatrix = ConvertMatrix(bone->mOffsetMatrix);
        }

        uint32_t index = boneMap[boneName].index;

        auto rgba = Color(Random::get_float_range(0.5f, 1.0f),
                          Random::get_float_range(0.5f, 1.0f),
                          Random::get_float_range(0.5f, 1.0f), 1.0f);
        for (uint32_t k = 0; k < bone->mNumWeights; ++k) {
          uint32_t vertexId = bone->mWeights[k].mVertexId;
          float weight = bone->mWeights[k].mWeight;
          boneData[vertexId].ids.push_back(index);
          boneData[vertexId].weights.push_back(weight);
          boneData[vertexId].color = rgba;
        }
      }

      for (uint32_t j = 0; j < mesh->mNumVertices; ++j) {
        AnimationVertex v;
        aiVector3D pos = mesh->mVertices[j];
        aiVector3D norm = aiVector3D(0, 1, 0);
        if (mesh->HasNormals()) {
          norm = mesh->mNormals[j];
        }
        aiVector3D tex = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][j]
                                                   : aiVector3D();

        v.position = glm::vec3(pos.x, pos.y, pos.z);
        v.normal = glm::vec3(norm.x, norm.y, norm.z);
        v.uv = glm::vec2(tex.x, tex.y);

        v.rgba = boneData[j].color;
        const auto &ids = boneData[j].ids;
        const auto &ws = boneData[j].weights;
        float temp = 0.f;
        assert(ids.size() <= 4);
        assert(ws.size() <= 4);
        for (int k = 0; k < 4; ++k) {
          v.boneIDs[k] = (k < ids.size()) ? float(ids[k]) : 0.0f;
          v.boneWeights[k] = (k < ws.size()) ? ws[k] : 0.0f;
        }

        v_array.animationVertices.push_back(v);
      }
      // indices
      for (uint32_t j = 0; j < mesh->mNumFaces; j++) {
        const aiFace &face = mesh->mFaces[j];
        for (uint32_t k = 0; k < face.mNumIndices; k++) {
          uint32_t index = face.mIndices[k];
          v_array.indices.push_back(vertexOffset + index);
        }
      }
      vertexOffset += mesh->mNumVertices;
    }
  } else if (scene->HasMeshes()) {
    // Iterate through the meshes
    for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
      const aiMesh *mesh = scene->mMeshes[i];
      // Process the mesh data
      for (uint32_t j = 0; j < mesh->mNumVertices; j++) {
        const aiVector3D &pos = mesh->mVertices[j];
        const aiVector3D &norm = mesh->mNormals[j];
        const aiVector3D &uv = mesh->mTextureCoords[0][j];

        Vertex v;
        v.position.x = pos.x;
        v.position.y = pos.y;
        v.position.z = pos.z;
        v.normal.x = norm.x;
        v.normal.y = norm.y;
        v.normal.z = norm.z;
        v.uv.x = uv.x;
        v.uv.y = uv.y;

        local_aabb.min.x =
            Math::Min(local_aabb.min.x, static_cast<float>(pos.x));
        local_aabb.min.y =
            Math::Min(local_aabb.min.y, static_cast<float>(pos.y));
        local_aabb.min.z =
            Math::Min(local_aabb.min.z, static_cast<float>(pos.z));
        local_aabb.max.x =
            Math::Max(local_aabb.max.x, static_cast<float>(pos.x));
        local_aabb.max.y =
            Math::Max(local_aabb.max.y, static_cast<float>(pos.y));
        local_aabb.max.z =
            Math::Max(local_aabb.max.z, static_cast<float>(pos.z));

        v_array.vertices.push_back(v);
      }
      // Process the indices
      for (uint32_t j = 0; j < mesh->mNumFaces; j++) {
        const aiFace &face = mesh->mFaces[j];
        for (uint32_t k = 0; k < face.mNumIndices; k++) {
          uint32_t index = face.mIndices[k];
          v_array.indices.push_back(index);
        }
      }
    }
  }

  v_array.indexCount = v_array.indices.size();
  auto viBuffer = CreateVertexIndexBuffer(v_array);
  modelData->vertexBuffer = viBuffer.first;
  modelData->indexBuffer = viBuffer.second;
}

void Model::load_from_vertex_array(const VertexArray &vArray) {
  auto modelData = GetModelData(this->data);
  modelData->v_array = vArray;
  auto &local_aabb = modelData->local_aabb;
  for (auto &v : vArray.vertices) {
    local_aabb.min.x = Math::Min(local_aabb.min.x, v.position.x);
    local_aabb.min.y = Math::Min(local_aabb.min.y, v.position.y);
    local_aabb.min.z = Math::Min(local_aabb.min.z, v.position.z);
    local_aabb.max.x = Math::Max(local_aabb.max.x, v.position.x);
    local_aabb.max.y = Math::Max(local_aabb.max.y, v.position.y);
    local_aabb.max.z = Math::Max(local_aabb.max.z, v.position.z);
  }
  auto viBuffer = CreateVertexIndexBuffer(vArray);
  modelData->vertexBuffer = viBuffer.first;
  modelData->indexBuffer = viBuffer.second;
}

void Model::load_sprite() { *this = RendererSystem::sprite; }
void Model::load_box() { *this = RendererSystem::box; }

AABB &Model::aabb() const {
  auto modelData = GetModelData(this->data);
  return modelData->local_aabb;
}

std::vector<Vertex> Model::all_vertex() const {
  auto modelData = GetModelData(this->data);
  auto &local_aabb = modelData->local_aabb;
  auto &v_array = modelData->v_array;
  auto &children = modelData->children;
  std::vector<Vertex> all;
  all.insert(all.end(), v_array.vertices.begin(), v_array.vertices.end());
  for (auto &child : children) {
    auto child_all = child->all_vertex();
    all.insert(all.end(), child_all.begin(), child_all.end());
  }
  return all;
}
std::vector<std::uint32_t> Model::all_indices() const {
  auto modelData = GetModelData(this->data);
  auto &local_aabb = modelData->local_aabb;
  auto &v_array = modelData->v_array;
  auto &children = modelData->children;
  std::vector<std::uint32_t> all;
  all.insert(all.end(), v_array.indices.begin(), v_array.indices.end());
  for (auto &child : children) {
    auto child_all = child->all_indices();
    all.insert(all.end(), child_all.begin(), child_all.end());
  }
  return all;
}

std::pair<px::Ptr<px::Buffer>, px::Ptr<px::Buffer>>
CreateVertexIndexBuffer(const VertexArray &vArray) {
  auto allocator = RendererSystem::GetPxRenderer()->GetAllocator();
  auto device = RendererSystem::GetPxRenderer()->GetDevice();
  size_t vertexBufferSize;
  bool isAnimation = false;
  if (vArray.animationVertices.size() > 0) {
    assert(vArray.vertices.empty());
    vertexBufferSize =
        vArray.animationVertices.size() * sizeof(AnimationVertex);
    isAnimation = true;
  } else {
    assert(vArray.animationVertices.empty());
    vertexBufferSize = vArray.vertices.size() * sizeof(Vertex);
  }
  px::Ptr<px::Buffer> vertexBuffer, indexBuffer;
  px::Buffer::CreateInfo vertexBufferInfo{};
  vertexBufferInfo.allocator = allocator;
  vertexBufferInfo.size = vertexBufferSize;
  vertexBufferInfo.usage = px::BufferUsage::Vertex;
  vertexBuffer = device->CreateBuffer(vertexBufferInfo);

  px::Buffer::CreateInfo indexBufferInfo{};
  indexBufferInfo.allocator = allocator;
  indexBufferInfo.size = vArray.indices.size() * sizeof(uint32_t);
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
      if (isAnimation) {
        memcpy(pMapped, vArray.animationVertices.data(),
               vArray.animationVertices.size() * sizeof(AnimationVertex));
      } else {
        memcpy(pMapped, vArray.vertices.data(),
               vArray.vertices.size() * sizeof(Vertex));
      }
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
    memcpy(pMapped, vArray.indices.data(), indexBufferInfo.size);
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

UniformData Model::bone_uniform_data() const {
  auto modelData = GetModelData(this->data);
  return modelData->boneUniformData;
}
void Model::play(float start) {
  time = start;
  load_bone_uniform(time);
}
void Model::update(float delta_time) {
  time += delta_time;
  load_bone_uniform(time);
}

void Model::load_bone_uniform(float start) {
  auto modelData = GetModelData(this->data);
  auto &skeletalAnimation = modelData->skeletalAnimation;
  for (auto &bone : skeletalAnimation.boneMap) {
    assert(bone.second.index <= skeletalAnimation.boneMap.size());
  }
  modelData->skeletalAnimation.Update(start);
  for (auto &bone : skeletalAnimation.boneMap) {
    assert(bone.second.index <= skeletalAnimation.boneMap.size());
  }
  auto matrices = skeletalAnimation.GetFinalBoneMatrices();
  auto &boneUniformData = modelData->boneUniformData;
  boneUniformData.clear();

  std::vector<glm::mat4> boneMatrices(matrices.size());
  int index = 0;
  for (auto &m : matrices) {
    boneMatrices[index] = m;
    index++;
  }
  boneUniformData.add_matrices(boneMatrices);
}
void SkeletalAnimation::Load(const aiScene *scn) {
  scene = scn;
  root = scene->mRootNode;
  globalInverseTransform = ConvertMatrix(root->mTransformation);
  globalInverseTransform = glm::inverse(globalInverseTransform);

  if (scene->mNumAnimations > 0) {
    aiAnimation *anim = scene->mAnimations[0];
    for (uint32_t i = 0; i < anim->mNumChannels; ++i) {
      aiNodeAnim *channel = anim->mChannels[i];
      nodeAnimMap[channel->mNodeName.C_Str()] = channel;
    }
  }
}
void SkeletalAnimation::Update(float timeInSeconds) {
  aiAnimation *anim = scene->mAnimations[0];
  float ticksPerSecond =
      anim->mTicksPerSecond != 0 ? anim->mTicksPerSecond : 25.0f;
  float timeInTicks = timeInSeconds * ticksPerSecond;
  float animationTime = Math::fmod(timeInTicks, anim->mDuration);

  ReadNodeHierarchy(animationTime, root, glm::mat4(1.0f));
}
void SkeletalAnimation::ReadNodeHierarchy(float animTime, aiNode *node,
                                          const glm::mat4 &parentTransform) {
  std::string nodeName = node->mName.C_Str();

  auto nodeTransform = ConvertMatrix(node->mTransformation);
  if (nodeAnimMap.contains(nodeName)) {
    nodeTransform = InterpolateTransform(nodeAnimMap[nodeName], animTime);
  }

  auto globalTransform = parentTransform * nodeTransform;

  if (boneMap.contains(nodeName)) {
    boneMap[nodeName].finalTransform = globalInverseTransform *
                                       globalTransform *
                                       boneMap[nodeName].offsetMatrix;
  }

  for (uint32_t i = 0; i < node->mNumChildren; ++i) {
    ReadNodeHierarchy(animTime, node->mChildren[i], globalTransform);
  }
}

uint32_t GetIndex(float time, const aiVectorKey *keys, uint32_t count) {
  for (uint32_t i = 0; i < count - 1; ++i) {
    if (time < keys[i + 1].mTime) {
      return i;
    }
  }
  return count - 1;
}

uint32_t GetIndex(float time, const aiQuatKey *keys, uint32_t count) {
  for (uint32_t i = 0; i < count - 1; ++i) {
    if (time < keys[i + 1].mTime) {
      return i;
    }
  }
  return count - 1;
}

glm::vec3 InterpolateVector3(const aiVectorKey *keys, uint32_t count,
                             float time, bool isScaling = false) {
  if (count == 0) {
    if (isScaling) {
      return glm::vec3(1, 1, 1);
    }
    return glm::vec3(0, 0, 0);
  }
  if (count == 1) {
    return glm::vec3(keys[0].mValue.x, keys[0].mValue.y, keys[0].mValue.z);
  }

  uint32_t i = GetIndex(time, keys, count);
  float t = float((time - keys[i].mTime) / (keys[i + 1].mTime - keys[i].mTime));
  auto k0 = glm::vec3(keys[i].mValue.x, keys[i].mValue.y, keys[i].mValue.z);
  auto k1 = glm::vec3(keys[i + 1].mValue.x, keys[i + 1].mValue.y,
                      keys[i + 1].mValue.z);
  return glm::mix(k0, k1, t);
};
glm::quat InterpolateQuat(const aiQuatKey *keys, uint32_t count, float time) {
  if (count == 0) {
    return glm::quat(1, 0, 0, 0);
  }
  if (count == 1) {
    return glm::quat(keys[0].mValue.w, keys[0].mValue.x, keys[0].mValue.y,
                     keys[0].mValue.z);
  }

  uint32_t i = GetIndex(time, keys, count);
  float t = float((time - keys[i].mTime) / (keys[i + 1].mTime - keys[i].mTime));
  glm::quat k0(keys[i].mValue.w, keys[i].mValue.x, keys[i].mValue.y,
               keys[i].mValue.z);
  glm::quat k1(keys[i + 1].mValue.w, keys[i + 1].mValue.x, keys[i + 1].mValue.y,
               keys[i + 1].mValue.z);
  auto q = glm::slerp(k0, k1, t);
  q = glm::normalize(q);
  return q;
};

glm::mat4 SkeletalAnimation::InterpolateTransform(aiNodeAnim *channel,
                                                  float time) {

  auto scaling = InterpolateVector3(channel->mScalingKeys,
                                    channel->mNumScalingKeys, time, true);
  auto rotation =
      InterpolateQuat(channel->mRotationKeys, channel->mNumRotationKeys, time);
  auto translation = InterpolateVector3(channel->mPositionKeys,
                                        channel->mNumPositionKeys, time);

  auto t = glm::translate(glm::mat4(1.0f), translation);
  auto r = glm::toMat4(rotation);
  auto s = glm::scale(glm::mat4(1.0f), scaling);

  auto m = t * r * s;
  return m;
}
std::vector<glm::mat4> SkeletalAnimation::GetFinalBoneMatrices() const {
  std::vector<glm::mat4> result(boneMap.size(), glm::mat4(1.0f));
  for (const auto &[name, info] : boneMap) {
    if (boneMap.contains(name)) {
      result[info.index] = info.finalTransform;
    } else {
      std::cerr << "[Warning] bone '" << name
                << "' has no finalTransform. Using identity.\n";
    }
  }
  return result;
}
} // namespace sinen