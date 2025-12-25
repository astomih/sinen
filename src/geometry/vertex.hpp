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
  Vec3 position;
  Vec3 normal;
  Vec2 uv;
  Color color = Color(1.0f);
};
struct AnimationVertex {
  Vec4 boneIDs;
  Vec4 boneWeights;
};
} // namespace sinen
#endif // !SINEN_VERTEX_HPP
