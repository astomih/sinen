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
#include <model/model.hpp>
#include <render/renderer.hpp>
namespace sinen {
enum class load_state { version, vertex, indices };

void model::load(std::string_view str, std::string_view name) {
  std::stringstream data;
  data << data_stream::open_as_string(asset_type::Model, str);
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

      vertex v;
      sscanf(line.data(), "%f %f %f %f %f %f %f %f %f %f %f %f\n",
             &v.position.x, &v.position.y, &v.position.z, &v.normal.x,
             &v.normal.y, &v.normal.z, &v.uv.x, &v.uv.y, &v.rgba.r, &v.rgba.g,
             &v.rgba.b, &v.rgba.a);

      local_aabb.min.x = std::min(local_aabb.min.x, v.position.x);
      local_aabb.min.y = std::min(local_aabb.min.y, v.position.y);
      local_aabb.min.z = std::min(local_aabb.min.z, v.position.z);
      local_aabb.max.x = std::max(local_aabb.max.x, v.position.x);
      local_aabb.max.y = std::max(local_aabb.max.y, v.position.y);
      local_aabb.max.z = std::max(local_aabb.max.z, v.position.z);

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
  v_array.indexCount = v_array.indices.size();
  render_system::add_vertex_array(v_array, name);
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
