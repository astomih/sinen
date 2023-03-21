#include <physics/primitive3.hpp>

namespace sinen {
void aabb::update_world(const vector3 &p, const vector3 &scale,
                        const aabb &local) {
  this->min = p + scale * local.min;
  this->max = p + scale * local.max;
}
} // namespace sinen
