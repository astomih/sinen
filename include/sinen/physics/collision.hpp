#ifndef SINEN_COLLISION_HPP
#define SINEN_COLLISION_HPP
#include "../math/vector3.hpp"
#include "primitive3.hpp"

namespace sinen {
class Collision {
public:
  static bool aabb_aabb(const AABB &a, const AABB &b);
  static bool obb_obb(const OBB &obb1, const OBB &obb2);

private:
  static float segment_length_on_separate_axis(const Vector3 &Sep,
                                               const Vector3 &e1,
                                               const Vector3 &e2,
                                               const Vector3 &e3);
  static float segment_length_on_separate_axis(const Vector3 &Sep,
                                               const Vector3 &e1,
                                               const Vector3 &e2);
};
} // namespace sinen
#endif // !SINEN_COLLISION_HPP
