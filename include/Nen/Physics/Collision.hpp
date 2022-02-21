#ifndef NEN_COLLISION_HPP
#define NEN_COLLISION_HPP
#include "../Math/Math.hpp"
#include "../Math/Vector3.hpp"
#include "primitive.hpp"

namespace nen {
class collision {
public:
  static bool aabb_aabb(const aabb &a, const aabb &b);
};
} // namespace nen
#endif