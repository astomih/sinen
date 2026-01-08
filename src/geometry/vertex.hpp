#ifndef SINEN_VERTEX_HPP
#define SINEN_VERTEX_HPP

#include <math/color/color.hpp>
#include <math/vector.hpp>

namespace sinen {
/**
 * @brief Vertex struct
 *
 */
struct Vertex {
  Vertex();
  Vertex(const Vec3 &position, const Vec3 &normal, const Vec2 &uv,
         const Color &color);
  Vec3 position;
  Vec3 normal;
  Vec2 uv;
  Color color;
};
} // namespace sinen
#endif // !SINEN_VERTEX_HPP
