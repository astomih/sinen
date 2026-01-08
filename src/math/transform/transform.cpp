#include "transform.hpp"
#include <math/matrix.hpp>
#include <math/quaternion.hpp>

namespace sinen {
Mat4 Transform::getWorldMatrix() const {
  const auto t = Mat4::translate(position);
  const auto r = Mat4::fromQuat(Quat::fromEuler(rotation));
  const auto s = Mat4::scale(scale);
  return t * r * s;
}
} // namespace sinen