#include <physics/primitive3.hpp>

namespace sinen {
void AABB::update_world(const Vector3 &p, const Vector3 &scale,
                        const AABB &local) {
  this->_min = p + scale * local._min;
  this->_max = p + scale * local._max;
}
} // namespace sinen
