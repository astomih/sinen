#include <physics/primitive3.hpp>

namespace sinen {
void aabb::update_world(const vector3 &p, const vector3 &scale,
                        const aabb &local) {
  this->_min = p + scale * local._min;
  this->_max = p + scale * local._max;
}
} // namespace sinen
