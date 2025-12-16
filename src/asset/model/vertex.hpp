#ifndef SINEN_VERTEX_HPP
#define SINEN_VERTEX_HPP
#include <string_view>

#include "../../math/color/color.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

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
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uv;
  Color color = Color(1.0f);
};
struct AnimationVertex {
  glm::vec4 boneIDs;
  glm::vec4 boneWeights;
};
} // namespace sinen
#endif // !SINEN_VERTEX_HPP
