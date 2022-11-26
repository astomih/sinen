#ifndef SINEN_VERTEX_HPP
#define SINEN_VERTEX_HPP
#include <string_view>

#include "../color/color.hpp"
#include "../math/vector2.hpp"
#include "../math/vector3.hpp"

namespace sinen {
/**
 * @brief Default vertices
 *
 */
struct vertex_default_shapes {
  static std::string_view sprite;
  static std::string_view box;
};
/**
 * @brief Vertex struct
 *
 */
struct vertex {
  vector3 position;
  vector3 normal;
  vector2 uv;
  color rgba = color(color(1, 1, 1, 1));
};
} // namespace sinen
#endif // !SINEN_VERTEX_HPP
