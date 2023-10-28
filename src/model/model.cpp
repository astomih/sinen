// std
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <sstream>
#include <string>
// internal
#include "../render/render_system.hpp"
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

void model::load(std::string_view str, std::string_view name) {
  std::stringstream data;
  data << data_stream::open_as_string(asset_type::Model, str);

  // Load gltf
  tinygltf::Model gltf_model;
  tinygltf::TinyGLTF gltf_ctx;
  std::string err;
  std::string warn;
  bool ret = gltf_ctx.LoadASCIIFromString(
      &gltf_model, &err, &warn, data.str().c_str(), data.str().size(), "", 0);
  if (!warn.empty()) {
    logger::warn(warn);
  }
  if (!err.empty()) {
    logger::error(err);
  }
  if (!ret) {
    logger::error("Failed to parse glTF");
  }

  v_array.vertices.clear();
  v_array.indices.clear();
  v_array.indexCount = 0;

  for (auto &mesh : gltf_model.meshes) {
    for (auto &primitive : mesh.primitives) {
      // Vertices
      auto &accessor = gltf_model.accessors[primitive.attributes["POSITION"]];
      auto &bufferView =
          gltf_model.bufferViews[accessor.bufferView]; // TODO: check
      auto &buffer = gltf_model.buffers[bufferView.buffer];
      auto data_ptr = buffer.data.data() + bufferView.byteOffset +
                      accessor.byteOffset; // TODO: check
      auto data_size = accessor.count * accessor.ByteStride(bufferView);
      auto data = reinterpret_cast<float *>(data_ptr);
      for (int i = 0; i < accessor.count; i++) {
        vertex v;
        v.position = vector3(data[0], data[1], data[2]);
        v_array.vertices.push_back(v);
        data += accessor.ByteStride(bufferView) / sizeof(float);
      }

      // Indices
      auto &indices_accessor = gltf_model.accessors[primitive.indices];
      auto &indices_bufferView =
          gltf_model.bufferViews[indices_accessor.bufferView]; // TODO: check
      auto &indices_buffer =
          gltf_model.buffers[indices_bufferView.buffer]; // TODO: check
      auto indices_data_ptr = indices_buffer.data.data() +
                              indices_bufferView.byteOffset +
                              indices_accessor.byteOffset; // TODO: check
      auto indices_data_size = indices_accessor.count *
                               indices_accessor.ByteStride(indices_bufferView);
      auto indices_data = reinterpret_cast<std::uint32_t *>(indices_data_ptr);
      for (int i = 0; i < indices_accessor.count; i++) {
        v_array.indices.push_back(indices_data[i]);
      }
    }
  }

  v_array.indexCount = v_array.indices.size();
  render_system::add_vertex_array(v_array, std::string(name));
}

std::vector<vertex> model::all_vertex() const {
  std::vector<vertex> all;
  all.insert(all.end(), v_array.vertices.begin(), v_array.vertices.end());
  for (auto &child : children) {
    auto child_all = child->all_vertex();
    all.insert(all.end(), child_all.begin(), child_all.end());
  }
  return all;
}
std::vector<std::uint32_t> model::all_indices() const {
  std::vector<std::uint32_t> all;
  all.insert(all.end(), v_array.indices.begin(), v_array.indices.end());
  for (auto &child : children) {
    auto child_all = child->all_indices();
    all.insert(all.end(), child_all.begin(), child_all.end());
  }
  return all;
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