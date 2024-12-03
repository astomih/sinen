// internal
#include <camera/camera.hpp>
#include <logger/logger.hpp>
#include <math/matrix4.hpp>
#include <render/renderer.hpp>
#include <window/window.hpp>

namespace sinen {
void Camera::lookat(const Vector3 &position, const Vector3 &target,
                    const Vector3 &up) {
  m_position = position;
  m_target = target;
  m_up = up;
  m_view = matrix4::lookat(m_position, m_target, m_up);
}
void Camera::perspective(float fov, float aspect, float near, float far) {
  m_projection = matrix4::perspective(Math::to_radians(fov), aspect, near, far);
}
void Camera::orthographic(float width, float height, float near, float far) {
  m_projection = matrix4::ortho(width, height, near, far);
}

} // namespace sinen
