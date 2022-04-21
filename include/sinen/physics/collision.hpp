#ifndef NEN_COLLISION_HPP
#define NEN_COLLISION_HPP
#include "../math/math.hpp"
#include "../math/vector3.hpp"
#include "primitive.hpp"

namespace nen {
class collision {
public:
  static bool aabb_aabb(const aabb &a, const aabb &b);
};
} // namespace nen
#endif