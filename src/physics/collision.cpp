#include <physics/collision.hpp>
namespace sinen {
bool collision::aabb_aabb(const aabb &a, const aabb &b) {
  return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
         (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
         (a.min.z <= b.max.z && a.max.z >= b.min.z);
}
bool aabb::intersects_aabb(const aabb &other) {
  return collision::aabb_aabb(*this, other);
}
} // namespace sinen
