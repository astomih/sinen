#ifndef SINEN_COLLISION_HPP
#define SINEN_COLLISION_HPP
#include "../math/math.hpp"
#include "../math/vector3.hpp"
#include "primitive.hpp"

namespace sinen {
class collision {
public:
  static bool aabb_aabb(const aabb &a, const aabb &b);
};
} // namespace sinen
#endif // !SINEN_COLLISION_HPP
