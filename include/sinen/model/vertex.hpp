#ifndef SINEN_VERTEX_HPP
#define SINEN_VERTEX_HPP
#include <string_view>

#include "../color/color.hpp"
#include "../math/vector2.hpp"
#include "../math/vector3.hpp"
#include "../math/vector4.hpp"

namespace sinen {
/**
 * @brief Default vertices
 *
 */
struct VertexDefaultShapes {
  static std::string_view sprite;
  static std::string_view box;
};
/**
 * @brief Vertex struct
 *
 */
struct Vertex {
  Vector3 position;
  Vector3 normal;
  Vector2 uv;
  Color rgba = Color(Color(1, 1, 1, 1));
};
struct AnimationVertex {
  Vector3 position;
  Vector3 normal;
  Vector2 uv;
  Color rgba = Color(Color(1, 1, 1, 1));
  Vector4 boneIDs;
  Vector4 boneWeights;
};
} // namespace sinen
#endif // !SINEN_VERTEX_HPP
