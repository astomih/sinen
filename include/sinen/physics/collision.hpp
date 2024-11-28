#ifndef SINEN_COLLISION_HPP
#define SINEN_COLLISION_HPP
#include "../math/vector3.hpp"
#include "primitive3.hpp"

namespace sinen {
class collision {
public:
  static bool aabb_aabb(const aabb &a, const aabb &b);
  static bool obb_obb(const OBB &obb1, const OBB &obb2);

private:
  static float segment_length_on_separate_axis(const vector3 &Sep,
                                               const vector3 &e1,
                                               const vector3 &e2,
                                               const vector3 &e3);
  static float segment_length_on_separate_axis(const vector3 &Sep,
                                               const vector3 &e1,
                                               const vector3 &e2);
};
} // namespace sinen
#endif // !SINEN_COLLISION_HPP
