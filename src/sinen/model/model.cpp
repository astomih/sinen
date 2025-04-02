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
#include "../render/px_renderer.hpp"
#include "../render/render_system.hpp"
#include "assimp/matrix4x4.h"
#include "assimp/postprocess.h"
#include "io/asset_type.hpp"
#include "math/matrix4.hpp"
#include "math/quaternion.hpp"
#include "model/vertex.hpp"
#include "model_data.hpp"
#include <io/data_stream.hpp>
#include <logger/logger.hpp>
#include <model/model.hpp>
#include <render/renderer.hpp>

#include <assimp/DefaultLogger.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace sinen {
enum class load_state { version, vertex, indices };
Model::Model() { data = std::make_shared<ModelData>(); };
void Model::load(std::string_view str) {
  auto modelData = GetModelData(this->data);
  auto &local_aabb = modelData->local_aabb;
  auto &v_array = modelData->v_array;
  std::stringstream data;
  data << DataStream::open_as_string(AssetType::Model, str);
  std::string str_name = str.data();
  if (str_name.ends_with(".sim")) {
    std::string line;
    load_state state = load_state::version;
    std::string version;
    std::size_t index;
    while (std::getline(data, line)) {
      switch (state) {
      case load_state::version:
        if (line.starts_with("version ")) {
          version = line.substr(8, 5);
        } else if (line.starts_with("begin vertex")) {
          state = load_state::vertex;
        } else {
          std::cerr << "invalid formats." << std::endl;
        }
        break;
      case load_state::vertex: {
        if (line.starts_with("end vertex"))
          break;
        if (line.starts_with("begin indices"))
          state = load_state::indices;

        Vertex v;
        sscanf(line.data(), "%f %f %f %f %f %f %f %f %f %f %f %f\n",
               &v.position.x, &v.position.y, &v.position.z, &v.normal.x,
               &v.normal.y, &v.normal.z, &v.uv.x, &v.uv.y, &v.rgba.r, &v.rgba.g,
               &v.rgba.b, &v.rgba.a);

        local_aabb._min.x = Math::Min(local_aabb._min.x, v.position.x);
        local_aabb._min.y = Math::Min(local_aabb._min.y, v.position.y);
        local_aabb._min.z = Math::Min(local_aabb._min.z, v.position.z);
        local_aabb._max.x = Math::Max(local_aabb._max.x, v.position.x);
        local_aabb._max.y = Math::Max(local_aabb._max.y, v.position.y);
        local_aabb._max.z = Math::Max(local_aabb._max.z, v.position.z);

        v_array.vertices.push_back(v);
      } break;
      case load_state::indices: {

        if (line.starts_with("end vertex"))
          break;

        uint32_t i;
        sscanf(line.data(), "%u\n", &i);
        v_array.indices.push_back(i);
      } break;
      default:
        break;
      }
    }
  } else if (str_name.ends_with(".gltf") || str_name.ends_with(".glb")) {

    auto fileName = DataStream::convert_file_path(AssetType::Model, str_name);
    // Assimp
    auto &importer = modelData->importer;
    modelData->scene =
        importer.ReadFile(fileName.c_str(), aiProcess_ValidateDataStructure);
    auto &scene = modelData->scene;
    if (!scene) {
      std::cerr << "Error loading model: " << importer.GetErrorString()
                << std::endl;
      return;
    }
    if (scene->HasAnimations()) {
      modelData->skeletalAnimation.Load(modelData->scene);
      uint32_t vertexOffset = 0;
      uint32_t numBone = 0;
      for (int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh *mesh = scene->mMeshes[i];

        struct BoneData {
          std::vector<uint32_t> ids;
          std::vector<float> weights;
        };
        std::unordered_map<uint32_t, BoneData> boneData;

        std::unordered_map<std::string, unsigned int> boneNameToIndex;
        auto &boneMap = modelData->skeletalAnimation.boneMap;
        for (unsigned int j = 0; j < mesh->mNumBones; j++) {
          aiBone *bone = mesh->mBones[j];
          std::string boneName = bone->mName.C_Str();

          if (!boneNameToIndex.contains(boneName)) {
            boneNameToIndex[boneName] = numBone++;
            boneMap[boneName].offsetMatrix = bone->mOffsetMatrix;
          }

          unsigned int index = boneNameToIndex[boneName];

          for (unsigned int k = 0; k < bone->mNumWeights; ++k) {
            unsigned int vertexId = bone->mWeights[k].mVertexId;
            float weight = bone->mWeights[k].mWeight;
            auto &data = boneData[vertexId];
            data.ids.push_back(index);
            data.weights.push_back(weight);
          }
        }

        for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
          AnimationVertex v;
          aiVector3D pos = mesh->mVertices[j];
          aiVector3D norm = aiVector3D(0, 1, 0);
          if (mesh->HasNormals()) {
            norm = mesh->mNormals[j];
          }
          aiVector3D tex = mesh->HasTextureCoords(0)
                               ? mesh->mTextureCoords[0][j]
                               : aiVector3D();

          v.position = Vector3(pos.x, pos.y, pos.z);
          v.normal = Vector3(norm.x, norm.y, norm.z);
          v.uv = Vector2(tex.x, tex.y);
          v.rgba = Color(1, 1, 1, 1);

          assert(boneData.contains(j));
          const auto &ids = boneData[j].ids;
          const auto &ws = boneData[j].weights;
          for (int k = 0; k < 4; ++k) {
            v.boneIDs[k] = (k < ids.size()) ? float(ids[k]) : 0.0f;
            v.boneWeights[k] = (k < ws.size()) ? ws[k] : 0.0f;
          }

          v_array.animationVertices.push_back(v);
        }
        // indices
        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
          const aiFace &face = mesh->mFaces[j];
          for (unsigned int k = 0; k < face.mNumIndices; k++) {
            uint32_t index = face.mIndices[k];
            v_array.indices.push_back(vertexOffset + index);
          }
        }
        vertexOffset += mesh->mNumVertices;
      }
    } else if (scene->HasMeshes()) {
      // Iterate through the meshes
      for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh *mesh = scene->mMeshes[i];
        // Process the mesh data
        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
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

          local_aabb._min.x =
              Math::Min(local_aabb._min.x, static_cast<float>(pos.x));
          local_aabb._min.y =
              Math::Min(local_aabb._min.y, static_cast<float>(pos.y));
          local_aabb._min.z =
              Math::Min(local_aabb._min.z, static_cast<float>(pos.z));
          local_aabb._max.x =
              Math::Max(local_aabb._max.x, static_cast<float>(pos.x));
          local_aabb._max.y =
              Math::Max(local_aabb._max.y, static_cast<float>(pos.y));
          local_aabb._max.z =
              Math::Max(local_aabb._max.z, static_cast<float>(pos.z));

          v_array.vertices.push_back(v);
        }
        // Process the indices
        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
          const aiFace &face = mesh->mFaces[j];
          for (unsigned int k = 0; k < face.mNumIndices; k++) {
            uint32_t index = face.mIndices[k];
            v_array.indices.push_back(index);
          }
        }
      }
    }
  } else {
    Logger::error("invalid formats.");
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
    local_aabb._min.x = Math::Min(local_aabb._min.x, v.position.x);
    local_aabb._min.y = Math::Min(local_aabb._min.y, v.position.y);
    local_aabb._min.z = Math::Min(local_aabb._min.z, v.position.z);
    local_aabb._max.x = Math::Max(local_aabb._max.x, v.position.x);
    local_aabb._max.y = Math::Max(local_aabb._max.y, v.position.y);
    local_aabb._max.z = Math::Max(local_aabb._max.z, v.position.z);
  }
  auto viBuffer = CreateVertexIndexBuffer(vArray);
  modelData->vertexBuffer = viBuffer.first;
  modelData->indexBuffer = viBuffer.second;
}

void Model::load_sprite() { *this = RendererImpl::sprite; }
void Model::load_box() { *this = RendererImpl::box; }

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
  auto allocator = RendererImpl::GetPxRenderer()->GetAllocator();
  auto device = RendererImpl::GetPxRenderer()->GetDevice();
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
  modelData->skeletalAnimation.Update(start);
  auto matrices = skeletalAnimation.GetFinalBoneMatrices();
  auto &boneUniformData = modelData->boneUniformData;
  boneUniformData.clear();

  std::vector<matrix4> boneMatrices(matrices.size());
  int index = 0;
  for (auto &m : matrices) {
    matrix4 mat;
    memcpy(&mat, &m, sizeof(matrix4));
    boneMatrices[index] = mat;
    index++;
  }
  boneUniformData.add_matrices(boneMatrices);
}
void SkeletalAnimation::Load(const aiScene *scn) {
  scene = scn;
  globalInverseTransform = scene->mRootNode->mTransformation;
  globalInverseTransform.Inverse();

  if (scene->mNumAnimations > 0) {
    aiAnimation *anim = scene->mAnimations[0];
    for (unsigned int i = 0; i < anim->mNumChannels; ++i) {
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

  ReadNodeHierarchy(animationTime, scene->mRootNode, aiMatrix4x4());
}
void SkeletalAnimation::ReadNodeHierarchy(float animTime, aiNode *node,
                                          const aiMatrix4x4 &parentTransform) {
  std::string nodeName = node->mName.C_Str();

  aiMatrix4x4 nodeTransform = node->mTransformation;
  if (nodeAnimMap.contains(nodeName)) {
    nodeTransform = InterpolateTransform(nodeAnimMap[nodeName], animTime);
  }

  aiMatrix4x4 globalTransform = parentTransform * nodeTransform;

  assert(boneMap.contains(nodeName) ||
         "Bone not found in boneMap. This may be due to a missing bone in the "
         "animation or a mismatch between the model and animation data.");
  auto &offset = boneMap[nodeName].offsetMatrix;
  auto &inverse = globalInverseTransform;
  auto &transform = globalTransform;
  boneMap[nodeName].finalTransform = inverse * transform * offset;

  for (unsigned int i = 0; i < node->mNumChildren; ++i) {
    ReadNodeHierarchy(animTime, node->mChildren[i], globalTransform);
  }
}
aiMatrix4x4 SkeletalAnimation::InterpolateTransform(aiNodeAnim *channel,
                                                    float time) {
  auto InterpolateVector3 = [&](const aiVectorKey *keys, unsigned int count,
                                float time) {
    if (count == 0) {
      return aiVector3D(0.f, 0.f, 0.f);
    }
    if (count == 1) {
      return keys[0].mValue;
    }

    for (unsigned int i = 0; i < count - 1; ++i) {
      if (time < keys[i + 1].mTime) {
        float t =
            float((time - keys[i].mTime) / (keys[i + 1].mTime - keys[i].mTime));
        return keys[i].mValue * (1.f - t) + keys[i + 1].mValue * t;
      }
    }
    return keys[count - 1].mValue;
  };

  auto InterpolateQuat = [&](const aiQuatKey *keys, unsigned int count,
                             float time) {
    if (count == 0) {
      return aiQuaternion(1, 0, 0, 0);
    }
    if (count == 1) {
      return keys[0].mValue;
    }

    for (unsigned int i = 0; i < count - 1; ++i) {
      if (time < keys[i + 1].mTime) {
        float t =
            float((time - keys[i].mTime) / (keys[i + 1].mTime - keys[i].mTime));
        aiQuaternion out;
        aiQuaternion::Interpolate(out, keys[i].mValue, keys[i + 1].mValue, t);
        out = out.Normalize();
        return out;
      }
    }
    return keys[count - 1].mValue;
  };

  aiVector3D scaling =
      InterpolateVector3(channel->mScalingKeys, channel->mNumScalingKeys, time);
  aiQuaternion rotation =
      InterpolateQuat(channel->mRotationKeys, channel->mNumRotationKeys, time);
  aiVector3D translation = InterpolateVector3(channel->mPositionKeys,
                                              channel->mNumPositionKeys, time);

  aiMatrix4x4 s;
  aiMatrix4x4::Scaling(scaling, s);
  aiMatrix4x4 r(rotation.GetMatrix());
  r = r.Transpose();
  aiMatrix4x4 t;
  aiMatrix4x4::Translation(translation, t);

  auto a = s * r * t;
  return a;
}
std::vector<aiMatrix4x4> SkeletalAnimation::GetFinalBoneMatrices() const {
  std::vector<aiMatrix4x4> result;
  for (const auto &[name, info] : boneMap) {
    result.push_back(info.finalTransform);
  }
  return result;
}
} // namespace sinen