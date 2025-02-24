// std
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
// internal
#include "../render/px_renderer.hpp"
#include "../render/render_system.hpp"
#include "io/asset_type.hpp"
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
    // Load gltf
    tinygltf::Model gltf_model;
    tinygltf::TinyGLTF gltf_ctx;
    std::string err;
    std::string warn;
    if (str_name.ends_with(".gltf")) {
      auto baseDir = DataStream::convert_file_path(AssetType::Model, "");
      bool ret = gltf_ctx.LoadASCIIFromString(
          &gltf_model, &err, &warn, data.str().c_str(), data.str().size(),
          baseDir.data(), 0);
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
          &gltf_model, &err, &warn,
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

    for (auto &mesh : gltf_model.meshes) {
      for (auto &primitive : mesh.primitives) {
        // Vertices
        {
          auto &positionAccessor =
              gltf_model.accessors[primitive.attributes["POSITION"]];
          auto &positionBufferView =
              gltf_model.bufferViews[positionAccessor.bufferView];
          auto &positionBuffer = gltf_model.buffers[positionBufferView.buffer];
          const float *positions = reinterpret_cast<const float *>(
              &positionBuffer.data[positionBufferView.byteOffset +
                                   positionAccessor.byteOffset]);

          auto &normalAccessor =
              gltf_model.accessors[primitive.attributes["NORMAL"]];
          auto &normalBufferView =
              gltf_model.bufferViews[normalAccessor.bufferView];
          auto &normalBuffer = gltf_model.buffers[normalBufferView.buffer];
          const float *normals = reinterpret_cast<const float *>(
              &normalBuffer.data[normalBufferView.byteOffset +
                                 normalAccessor.byteOffset]);

          auto &uvAccessor =
              gltf_model.accessors[primitive.attributes["TEXCOORD_0"]];
          auto &uvBufferView = gltf_model.bufferViews[uvAccessor.bufferView];
          auto &uvBuffer = gltf_model.buffers[uvBufferView.buffer];
          const float *uvs = reinterpret_cast<const float *>(
              &uvBuffer.data[uvBufferView.byteOffset + uvAccessor.byteOffset]);

          for (size_t i = 0; i < positionAccessor.count; ++i) {
            v_array.vertices.push_back(Vertex{
                .position = Vector3(positions[3 * i + 0], positions[3 * i + 1],
                                    positions[3 * i + 2]),
                .normal = Vector3(normals[3 * i + 0], normals[3 * i + 1],
                                  normals[3 * i + 2]),
                .uv = Vector2(uvs[2 * i + 0], uvs[2 * i + 1]),
            });
          }
        }

        // Indices
        {
          auto &accessor = gltf_model.accessors[primitive.indices];
          auto buffer_view = gltf_model.bufferViews[accessor.bufferView];
          auto buffer = gltf_model.buffers[buffer_view.buffer];
          const unsigned short *indices_data_fromgltf =
              reinterpret_cast<const unsigned short *>(
                  &buffer.data[buffer_view.byteOffset + accessor.byteOffset]);
          for (size_t i = 0; i < accessor.count; ++i) {
            v_array.indices.push_back(indices_data_fromgltf[i]);
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
  auto vertexBufferSize = vArray.vertices.size() * sizeof(Vertex);

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
      memcpy(pMapped, vArray.vertices.data(), vertexBufferSize);
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