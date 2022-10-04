#include "../main/get_system.hpp"
#include "../render/render_system.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <sstream>
#include <string>

#include <io/dstream.hpp>
#include <model/model.hpp>
#include <render/renderer.hpp>

namespace sinen {
enum class load_state { version, vertex, indices };

void model::load(std::string_view str, std::string_view name) {
  std::stringstream data;
  data << dstream::open_as_string(asset_type::Model, str);
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

      m_aabb.min.x = std::min(m_aabb.min.x, v.position.x);
      m_aabb.min.y = std::min(m_aabb.min.y, v.position.y);
      m_aabb.min.z = std::min(m_aabb.min.z, v.position.z);
      m_aabb.max.x = std::max(m_aabb.max.x, v.position.x);
      m_aabb.max.y = std::max(m_aabb.max.y, v.position.y);
      m_aabb.max.z = std::max(m_aabb.max.z, v.position.z);

      m_array.vertices.push_back(v);
    } break;
    case load_state::indices: {

      if (line.starts_with("end vertex"))
        break;

      uint32_t i;
      sscanf(line.data(), "%u\n", &i);
      m_array.indices.push_back(i);
    } break;
    default:
      break;
    }
  }
  m_array.indexCount = m_array.indices.size();
  get_renderer().add_vertex_array(m_array, name);
}
} // namespace sinen