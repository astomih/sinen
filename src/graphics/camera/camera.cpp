#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

// internal
#include <graphics/camera/camera.hpp>
#include <graphics/graphics.hpp>
#include <platform/window/window.hpp>

namespace sinen {
void Camera::LookAt(const glm::vec3 &position, const glm::vec3 &target,
                    const glm::vec3 &up) {
  m_position = position;
  m_target = target;
  m_up = up;
  m_view = glm::lookAt(glm::vec3(m_position.x, m_position.y, m_position.z),
                       glm::vec3(m_target.x, m_target.y, m_target.z),
                       glm::vec3(m_up.x, m_up.y, m_up.z));
  update_frustum = true;
}
void Camera::Perspective(float fov, float aspect, float near, float far) {
  m_projection = glm::perspective(glm::radians(fov), aspect, near, far);
  update_frustum = true;
}
void Camera::Orthographic(float width, float height, float near, float far) {
  m_projection = glm::ortho(0.f, width, 0.f, height, near, far);
  update_frustum = true;
}

static Frustum ExtractFrustumPlanes(const glm::mat4 &vp) {
  Frustum f;

  f.planes[0] = glm::vec4(vp[0][3] + vp[0][0], vp[1][3] + vp[1][0],
                          vp[2][3] + vp[2][0], vp[3][3] + vp[3][0]); // Left
  f.planes[1] = glm::vec4(vp[0][3] - vp[0][0], vp[1][3] - vp[1][0],
                          vp[2][3] - vp[2][0], vp[3][3] - vp[3][0]); // Right
  f.planes[2] = glm::vec4(vp[0][3] - vp[0][1], vp[1][3] - vp[1][1],
                          vp[2][3] - vp[2][1], vp[3][3] - vp[3][1]); // Top
  f.planes[3] = glm::vec4(vp[0][3] + vp[0][1], vp[1][3] + vp[1][1],
                          vp[2][3] + vp[2][1], vp[3][3] + vp[3][1]); // Bottom
  f.planes[4] = glm::vec4(vp[0][3] + vp[0][2], vp[1][3] + vp[1][2],
                          vp[2][3] + vp[2][2], vp[3][3] + vp[3][2]); // Near
  f.planes[5] = glm::vec4(vp[0][3] - vp[0][2], vp[1][3] - vp[1][2],
                          vp[2][3] - vp[2][2], vp[3][3] - vp[3][2]); // Far

  // Normalize
  for (int i = 0; i < 6; i++) {
    float length = glm::length(glm::vec3(f.planes[i]));
    f.planes[i] /= length;
  }

  return f;
}
bool Camera::IsAABBInFrustum(const AABB &aabb) {
  if (update_frustum) {
    frustum = ExtractFrustumPlanes(m_projection * m_view);
    update_frustum = false;
  }
  for (int i = 0; i < 6; ++i) {
    const glm::vec4 &plane = frustum.planes[i];
    glm::vec3 normal = glm::vec3(plane);
    float d = plane.w;

    glm::vec3 p = glm::vec3((normal.x > 0 ? aabb.max.x : aabb.min.x),
                            (normal.y > 0 ? aabb.max.y : aabb.min.y),
                            (normal.z > 0 ? aabb.max.z : aabb.min.z));

    if (glm::dot(normal, p) + d < 0) {
      return false;
    }
  }

  return true;
}

} // namespace sinen
