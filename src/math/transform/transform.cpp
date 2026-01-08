#include "transform.hpp"
#include <math/matrix.hpp>
#include <math/quaternion.hpp>

namespace sinen {
Mat4 Transform::getWorldMatrix() const {
  const auto t = Mat4::create_translation(position);
  const auto r = Mat4::create_from_quaternion(Quaternion::from_euler(rotation));
  const auto s = Mat4::create_scale(scale);
  return t * r * s;
}
} // namespace sinen