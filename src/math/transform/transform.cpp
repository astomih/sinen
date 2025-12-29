#include "transform.hpp"
#include <math/matrix.hpp>
#include <math/quaternion.hpp>

namespace sinen {
Mat4 Transform::getWorldMatrix() const {

  const auto t = glm::translate(Mat4(1.0f), position);
  const auto rotationX =
      glm::angleAxis(glm::radians(rotation.x), Vec3(1.0f, 0.0f, 0.0f));
  const auto rotationY =
      glm::angleAxis(glm::radians(rotation.y), Vec3(0.0f, 1.0f, 0.0f));
  const auto rotationZ =
      glm::angleAxis(glm::radians(rotation.z), Vec3(0.0f, 0.0f, 1.0f));
  const auto r = glm::toMat4(rotationX * rotationY * rotationZ);
  const auto s = glm::scale(Mat4(1.0f), scale);
  return glm::transpose(t * r * s);
}
} // namespace sinen