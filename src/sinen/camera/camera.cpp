#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

// internal
#include <camera/camera.hpp>
#include <logger/logger.hpp>
#include <render/renderer.hpp>
#include <window/window.hpp>

namespace sinen {
void Camera::lookat(const glm::vec3 &position, const glm::vec3 &target,
                    const glm::vec3 &up) {
  m_position = position;
  m_target = target;
  m_up = up;
  m_view = glm::lookAt(glm::vec3(m_position.x, m_position.y, m_position.z),
                       glm::vec3(m_target.x, m_target.y, m_target.z),
                       glm::vec3(m_up.x, m_up.y, m_up.z));
}
void Camera::perspective(float fov, float aspect, float near, float far) {
  m_projection = glm::perspective(glm::radians(fov), aspect, near, far);
}
void Camera::orthographic(float width, float height, float near, float far) {
  m_projection = glm::ortho(0.f, width, 0.f, height, near, far);
}

} // namespace sinen
