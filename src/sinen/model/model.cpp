// std
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <variant>

// internal
#include "../render/px_renderer.hpp"
#include "../render/render_system.hpp"
#include "io/asset_type.hpp"
#include "math/matrix4.hpp"
#include "math/quaternion.hpp"
#include "model/vertex.hpp"
#include "model_data.hpp"
#include <io/data_stream.hpp>
#include <logger/logger.hpp>
#include <model/model.hpp>
#include <render/renderer.hpp>

// gltf
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_USE_RAPIDJSON
#define TINYGLTF_NO_INCLUDE_RAPIDJSON
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <tiny_gltf.h>

namespace sinen {
enum class load_state { version, vertex, indices };
static matrix4 GetLocalTransform(const tinygltf::Node &node) {
  matrix4 m = matrix4::identity;
  if (node.matrix.size() == 16) {
    memcpy(&m.mat, node.matrix.data(), sizeof(matrix4));
    return m;
  }

  matrix4 t = matrix4::identity, r = matrix4::identity, s = matrix4::identity;
  assert(node.scale.size() <= 3);
  assert(node.rotation.size() <= 4);
  assert(node.translation.size() <= 3);

  if (node.scale.size() == 3) {
    float x = static_cast<float>(node.scale[0]);
    float y = static_cast<float>(node.scale[1]);
    float z = static_cast<float>(node.scale[2]);
    s = matrix4::create_scale(Vector3(x, y, z));
  }
  if (node.rotation.size() == 4) {
    float x = static_cast<float>(node.rotation[0]);
    float y = static_cast<float>(node.rotation[1]);
    float z = static_cast<float>(node.rotation[2]);
    float w = static_cast<float>(node.rotation[3]);
    r = matrix4::create_from_quaternion(Quaternion(x, y, z, w));
  }
  if (node.translation.size() == 3) {
    float x = static_cast<float>(node.translation[0]);
    float y = static_cast<float>(node.translation[1]);
    float z = static_cast<float>(node.translation[2]);
    t = matrix4::create_translation(Vector3(x, y, z));
  }
  m = s * r * t;

  return m;
}
static void ComputeNodeMatrices(const tinygltf::Model &model, int nodeIndex,
                                std::unordered_set<int> &nodesToUpdate,
                                const matrix4 &parentTransform,
                                std::vector<matrix4> &outModelMats) {
  if (nodesToUpdate.contains(nodeIndex)) {
    return;
  }
  nodesToUpdate.insert(nodeIndex);
  const tinygltf::Node &node = model.nodes[nodeIndex];
  auto localMat = GetLocalTransform(node);
  auto globalMat = parentTransform * localMat;

  outModelMats[nodeIndex] = globalMat;

  for (size_t i = 0; i < node.children.size(); i++) {
    ComputeNodeMatrices(model, node.children[i], nodesToUpdate, globalMat,
                        outModelMats);
  }
}

void LoadSkinningData(const tinygltf::Model &model,
                      const tinygltf::Primitive &primitive,
                      std::vector<Vector4> &boneIDs,
                      std::vector<Vector4> &boneWeights) {
  if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
    const tinygltf::Accessor &jointAccessor =
        model.accessors[primitive.attributes.at("JOINTS_0")];
    const tinygltf::BufferView &jointBufferView =
        model.bufferViews[jointAccessor.bufferView];
    const tinygltf::Buffer &jointBuffer = model.buffers[jointBufferView.buffer];
    assert(jointAccessor.type == TINYGLTF_TYPE_VEC4);
    size_t byteStride = jointAccessor.ByteStride(jointBufferView);

    if (jointAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {

      const auto *jointData = reinterpret_cast<const unsigned short *>(
          &jointBuffer
               .data[jointBufferView.byteOffset + jointAccessor.byteOffset]);

      if (byteStride == 0) {
        byteStride = sizeof(uint16_t) * 4;
      }

      for (size_t i = 0; i < jointAccessor.count; ++i) {
        size_t index = i * (byteStride / sizeof(uint16_t));
        float j0 = static_cast<float>(jointData[index + 0]);
        float j1 = static_cast<float>(jointData[index + 1]);
        float j2 = static_cast<float>(jointData[index + 2]);
        float j3 = static_cast<float>(jointData[index + 3]);
        boneIDs.push_back(Vector4(j0, j1, j2, j3));
      }
    } else if (jointAccessor.componentType ==
               TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
      if (byteStride == 0) {
        byteStride = sizeof(uint8_t) * 4;
      }
      const auto *jointData = reinterpret_cast<const uint8_t *>(
          &jointBuffer
               .data[jointBufferView.byteOffset + jointAccessor.byteOffset]);

      for (size_t i = 0; i < jointAccessor.count; ++i) {
        size_t index = i * (byteStride / sizeof(uint8_t));
        float j0 = static_cast<float>(jointData[index + 0]);
        float j1 = static_cast<float>(jointData[index + 1]);
        float j2 = static_cast<float>(jointData[index + 2]);
        float j3 = static_cast<float>(jointData[index + 3]);
        boneIDs.push_back(Vector4(j0, j1, j2, j3));
      }
    }
  }

  if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
    const tinygltf::Accessor &accessor =
        model.accessors[primitive.attributes.at("WEIGHTS_0")];
    const tinygltf::BufferView &bufferView =
        model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

    assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
    assert(accessor.type == TINYGLTF_TYPE_VEC4);

    const float *weightData = reinterpret_cast<const float *>(
        &buffer.data[bufferView.byteOffset + accessor.byteOffset]);

    for (size_t i = 0; i < accessor.count; ++i) {
      boneWeights.push_back(
          Vector4(weightData[i * 4 + 0], weightData[i * 4 + 1],
                  weightData[i * 4 + 2], weightData[i * 4 + 3]));
    }
  }

  if (boneIDs.empty() && boneWeights.empty()) {
    auto &positionAccessor =
        model.accessors[primitive.attributes.at("POSITION")];
    for (size_t i = 0; i < positionAccessor.count; ++i) {
      boneIDs.push_back(Vector4(0, 0, 0, 0));
      boneWeights.push_back(Vector4(1,1,1,1));
    }
  }
}
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
    auto &model = modelData->model;
    // Load gltf
    tinygltf::TinyGLTF gltf_ctx;
    std::string err;
    std::string warn;
    if (str_name.ends_with(".gltf")) {
      auto baseDir = DataStream::convert_file_path(AssetType::Model, "");
      bool ret =
          gltf_ctx.LoadASCIIFromString(&model, &err, &warn, data.str().c_str(),
                                       data.str().size(), baseDir.data(), 0);
      if (!warn.empty()) {
        Logger::warn(warn);
      }
      if (!err.empty()) {
        Logger::error(err);
      }
      if (!ret) {
        Logger::error("Failed to parse glTF");
      }
    }
    if (str_name.ends_with(".glb")) {
      bool ret = gltf_ctx.LoadBinaryFromFile(
          &model, &err, &warn,
          DataStream::convert_file_path(AssetType::Model, str_name));
      if (!warn.empty()) {
        Logger::warn(warn);
      }
      if (!err.empty()) {
        Logger::error(err);
      }
      if (!ret) {
        Logger::error("Failed to parse glTF");
      }
    }

    bool isAnimation = false;
    if (!model.animations.empty()) {
      isAnimation = true;
    }

    for (auto &mesh : model.meshes) {
      for (auto &primitive : mesh.primitives) {
        // Vertices
        {
          auto &positionAccessor =
              model.accessors[primitive.attributes["POSITION"]];
          assert(positionAccessor.componentType ==
                 TINYGLTF_COMPONENT_TYPE_FLOAT);
          assert(positionAccessor.type == TINYGLTF_TYPE_VEC3);
          auto &positionBufferView =
              model.bufferViews[positionAccessor.bufferView];
          assert(positionAccessor.componentType ==
                 TINYGLTF_COMPONENT_TYPE_FLOAT);
          auto &positionBuffer = model.buffers[positionBufferView.buffer];
          const float *positions = reinterpret_cast<const float *>(
              &positionBuffer.data[positionBufferView.byteOffset +
                                   positionAccessor.byteOffset]);

          auto &normalAccessor =
              model.accessors[primitive.attributes["NORMAL"]];
          auto &normalBufferView = model.bufferViews[normalAccessor.bufferView];
          auto &normalBuffer = model.buffers[normalBufferView.buffer];
          const float *normals = reinterpret_cast<const float *>(
              &normalBuffer.data[normalBufferView.byteOffset +
                                 normalAccessor.byteOffset]);

          auto &uvAccessor =
              model.accessors[primitive.attributes["TEXCOORD_0"]];
          auto &uvBufferView = model.bufferViews[uvAccessor.bufferView];
          auto &uvBuffer = model.buffers[uvBufferView.buffer];
          std::variant<const float *, const uint16_t *> uvs;
          if (uvAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
            uvs = reinterpret_cast<const float *>(
                &uvBuffer
                     .data[uvBufferView.byteOffset + uvAccessor.byteOffset]);
          } else if (uvAccessor.componentType ==
                     TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
            uvs = reinterpret_cast<const uint16_t *>(
                &uvBuffer
                     .data[uvBufferView.byteOffset + uvAccessor.byteOffset]);
          }

          std::vector<Vector4> boneIDs, boneWeights;
          if (isAnimation)
            LoadSkinningData(model, primitive, boneIDs, boneWeights);

          for (size_t i = 0; i < positionAccessor.count; ++i) {
            if (isAnimation) {
              AnimationVertex v;
              v.position = Vector3(positions[3 * i + 0], positions[3 * i + 1],
                                   positions[3 * i + 2]);
              v.normal = Vector3(normals[3 * i + 0], normals[3 * i + 1],
                                 normals[3 * i + 2]);
              if (uvAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                auto uv = std::get<const float *>(uvs);
                v.uv = Vector2(uv[2 * i + 0], uv[2 * i + 1]);
              } else if (uvAccessor.componentType ==
                         TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                auto uv = std::get<const uint16_t *>(uvs);
                v.uv = Vector2(uv[2 * i + 0], uv[2 * i + 1]);
              }
              v.boneIDs = boneIDs[i];
              v.boneWeights = boneWeights[i];
              v_array.animationVertices.push_back(v);
            } else {
              Vertex v;
              v.position = Vector3(positions[3 * i + 0], positions[3 * i + 1],
                                   positions[3 * i + 2]);
              v.normal = Vector3(normals[3 * i + 0], normals[3 * i + 1],
                                 normals[3 * i + 2]);
              if (uvAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                auto uv = std::get<const float *>(uvs);
                v.uv = Vector2(uv[2 * i + 0], uv[2 * i + 1]);
              } else if (uvAccessor.componentType ==
                         TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                auto uv = std::get<const uint16_t *>(uvs);
                v.uv = Vector2(uv[2 * i + 0], uv[2 * i + 1]);
              }
              v_array.vertices.push_back(v);
            }
          }
        }

        // Indices
        {
          auto &accessor = model.accessors[primitive.indices];
          auto buffer_view = model.bufferViews[accessor.bufferView];
          auto buffer = model.buffers[buffer_view.buffer];
          const unsigned short *indices_data_fromgltf =
              reinterpret_cast<const unsigned short *>(
                  &buffer.data[buffer_view.byteOffset + accessor.byteOffset]);
          for (size_t i = 0; i < accessor.count; ++i) {
            v_array.indices.push_back(indices_data_fromgltf[i]);
          }
        }
      }
    }

    for (auto &skin : model.skins) {
      const tinygltf::Accessor &invAccessor =
          model.accessors[skin.inverseBindMatrices];
      const tinygltf::BufferView &invBufferView =
          model.bufferViews[invAccessor.bufferView];
      const tinygltf::Buffer &invBuffer = model.buffers[invBufferView.buffer];

      const float *matrixData = reinterpret_cast<const float *>(
          &invBuffer.data[invBufferView.byteOffset]);
      assert(invAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
      assert(invAccessor.type == TINYGLTF_TYPE_MAT4);

      for (size_t i = 0; i < invAccessor.count; ++i) {
        matrix4 m;
        memcpy(&m.mat, &matrixData[i * 16], sizeof(matrix4));
        inverse_bind_matrices.push_back(m);
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
  auto &model = modelData->model;

  // Animation
  for (auto &animation : model.animations) {
    for (auto &channel : animation.channels) {
      auto &sampler = animation.samplers[channel.sampler];
      assert(sampler.interpolation == "LINEAR");
      auto &inputAccessor = model.accessors[sampler.input];

      auto &outputAccessor = model.accessors[sampler.output];
      auto &inputBufferView = model.bufferViews[inputAccessor.bufferView];
      auto &outputBufferView = model.bufferViews[outputAccessor.bufferView];
      auto &inputBuffer = model.buffers[inputBufferView.buffer];
      auto &outputBuffer = model.buffers[outputBufferView.buffer];
      auto &node = model.nodes[channel.target_node];
      assert(inputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
      assert(inputAccessor.type == TINYGLTF_TYPE_SCALAR);
      assert(outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
      const float *inputData = reinterpret_cast<const float *>(
          &inputBuffer
               .data[inputBufferView.byteOffset + inputAccessor.byteOffset]);
      const auto *outputData = reinterpret_cast<const float *>(
          &outputBuffer
               .data[outputBufferView.byteOffset + outputAccessor.byteOffset]);
      // Find the frame
      size_t t0, t1;
      for (t0 = 0; t0 < inputAccessor.count - 1; t0++) {
        if (inputData[t0] <= start && start < inputData[t0 + 1]) {
          break;
        }
      }
      if (inputAccessor.count > 2) {
        t1 = t0 + 1;
      } else
        t1 = t0;

      if (channel.target_path == "rotation") {
        Quaternion q0(outputData[4 * t0 + 0], outputData[4 * t0 + 1],
                      outputData[4 * t0 + 2], outputData[4 * t0 + 3]);
        Quaternion q1(outputData[4 * t1 + 0], outputData[4 * t1 + 1],
                      outputData[4 * t1 + 2], outputData[4 * t1 + 3]);
        float t = (start - inputData[t0]) / (inputData[t1] - inputData[t0]);
        Quaternion q = Quaternion::slerp(q0, q1, t);
        node.rotation[0] = q.x;
        node.rotation[1] = q.y;
        node.rotation[2] = q.z;
        node.rotation[3] = q.w;
      } else if (channel.target_path == "translation") {
        Vector3 v0(outputData[3 * t0 + 0], outputData[3 * t0 + 1],
                   outputData[3 * t0 + 2]);
        Vector3 v1(outputData[3 * t1 + 0], outputData[3 * t1 + 1],
                   outputData[3 * t1 + 2]);
        float t = (start - inputData[t0]) / (inputData[t1] - inputData[t0]);
        Vector3 v = Vector3::lerp(v0, v1, t);
        node.translation[0] = v.x;
        node.translation[1] = v.y;
        node.translation[2] = v.z;
      } else if (channel.target_path == "scale") {
        Vector3 s0(outputData[3 * t0 + 0], outputData[3 * t0 + 1],
                   outputData[3 * t0 + 2]);
        Vector3 s1(outputData[3 * t1 + 0], outputData[3 * t1 + 1],
                   outputData[3 * t1 + 2]);
        float t = (start - inputData[t0]) / (inputData[t1] - inputData[t0]);
        Vector3 s = Vector3::lerp(s0, s1, t);
        if (node.scale.size() < 3) {
          node.scale.resize(3);
        }
        node.scale[0] = s.x;
        node.scale[1] = s.y;
        node.scale[2] = s.z;
      }
    }
  }

  // Compute node matrices
  auto &boneUniformData = modelData->boneUniformData;
  boneUniformData.clear();

  std::vector<matrix4> nodeMatrices(model.nodes.size(), matrix4::identity);
  std::unordered_set<int> nodesToUpdate;
  if (model.skins.empty()) {
    for (int i = 0; i < model.nodes.size(); i++) {
      ComputeNodeMatrices(model, i, nodesToUpdate, matrix4::identity,
                          nodeMatrices);
    }
    boneUniformData.add_matrices(nodeMatrices);
    return;
  }
  const tinygltf::Skin &skin = model.skins[0];

  for (int jointNode : skin.joints) {
    ComputeNodeMatrices(model, jointNode, nodesToUpdate, matrix4::identity,
                        nodeMatrices);
  }

  auto &joints = skin.joints;
  std::vector<matrix4> jointMatrices(joints.size());
  for (size_t i = 0; i < joints.size(); i++) {
    int jointNodeIndex = joints[i];
    auto nodeMat = nodeMatrices[jointNodeIndex];
    auto ibm = inverse_bind_matrices[i];
    auto jointMat = nodeMat * ibm;
    jointMatrices[i] = jointMat;
  }

  boneUniformData.add_matrices(jointMatrices);
}
} // namespace sinen

namespace tinygltf {
bool LoadImageData(Image *image, const int image_idx, std::string *err,
                   std::string *warn, int req_width, int req_height,
                   const unsigned char *bytes, int size, void *) {
  bool result = false;
  return result;
}

bool WriteImageData(const std::string *basepath, const std::string *filename,
                    Image *image, bool embedImages, void *) {
  bool result = false;
  return result;
}

} // namespace tinygltf