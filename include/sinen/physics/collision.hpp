#ifndef SINEN_COLLISION_HPP
#define SINEN_COLLISION_HPP
#include <glm/vec3.hpp>

namespace sinen {
struct AABB {
  glm::vec3 min;
  glm::vec3 max;
  AABB() : min(glm::vec3(0.0f)), max(glm::vec3(0.0f)) {}
  ~AABB() = default;
  /**
   * @brief Construct a new aabb object
   *
   * @param min  min point
   * @param max  max point
   */
  AABB(const glm::vec3 &min, const glm::vec3 max) : min(min), max(max) {}
  void updateWorld(const glm::vec3 &p, const glm::vec3 &scale,
                   const AABB &local);
};
/**
 * @brief OBBox(Oriented Bounding Box) class
 *
 */
struct OBB {
  OBB() = default;
  ~OBB() = default;
  // Position
  glm::vec3 p;
  // Direct vectors
  glm::vec3 v[3];
  // Axis lengths
  float a[3];
};
class Collision {
public:
  static bool AABBvsAABB(const AABB &a, const AABB &b);
  static bool OBBvsOBB(const OBB &obb1, const OBB &obb2);

private:
  static float SegmentLengthOnSeparateAxis(const glm::vec3 &Sep,
                                           const glm::vec3 &e1,
                                           const glm::vec3 &e2,
                                           const glm::vec3 &e3);
  static float SegmentLengthOnSeparateAxis(const glm::vec3 &Sep,
                                           const glm::vec3 &e1,
                                           const glm::vec3 &e2);
};
} // namespace sinen
#endif // !SINEN_COLLISION_HPP
