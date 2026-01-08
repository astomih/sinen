#ifndef SINEN_COLLISION_HPP
#define SINEN_COLLISION_HPP
#include <math/geometry/bbox.hpp>

namespace sinen {
class Collision {
public:
  static bool aabBvsAabb(const AABB &a, const AABB &b);
  static bool obBvsObb(const OBB &obb1, const OBB &obb2);

private:
  static float segmentLengthOnSeparateAxis(const Vec3 &Sep, const Vec3 &e1,
                                           const Vec3 &e2, const Vec3 &e3);
  static float segmentLengthOnSeparateAxis(const Vec3 &Sep, const Vec3 &e1,
                                           const Vec3 &e2);
};
} // namespace sinen
#endif // !SINEN_COLLISION_HPP
