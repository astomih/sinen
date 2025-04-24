#ifndef SINEN_CAMERA_HPP
#define SINEN_CAMERA_HPP
#include <glm/vec3.hpp>

#include <glm/mat4x4.hpp>

#include "../physics/collision.hpp"

namespace sinen {
struct Frustum {
  glm::vec4 planes[6]; // x, y, z, w: ax + by + cz + d = 0
};
/**
 * @brief Camera class
 *
 */
class Camera {
public:
  /**
   * @brief Move to position, look at a target, and set up vector
   *
   * @param position
   * @param target
   * @param up
   */
  void lookat(const glm::vec3 &position, const glm::vec3 &target,
              const glm::vec3 &up);
  /**
   * @brief Set Perspective projection
   *
   * @param fov Field of view
   * @param aspect Aspect ratio
   * @param near Near plane
   * @param far Far plane
   */
  void perspective(float fov, float aspect, float near, float far);
  /**
   * @brief Set Orthographic projection
   *
   * @param width Width of the orthographic projection
   * @param height Height of the orthographic projection
   * @param near Near plane
   * @param far Far plane
   */
  void orthographic(float width, float height, float near, float far);

  /**
   * @brief Check if AABB is in frustum
   *
   * @param aabb AABB to check world space AABB
   * @return true AABB is in frustum
   * @return false AABB is not in frustum
   */
  bool is_aabb_in_frustum(const AABB &aabb);
  /**
   * @brief Get the position of camera
   *
   * @return glm::vec3&
   */
  glm::vec3 &position() { return m_position; }
  /**
   * @brief Get the target of camera
   *
   * @return glm::vec3&
   */
  glm::vec3 &target() { return m_target; }
  /**
   * @brief Get the up vector of camera
   *
   * @return glm::vec3&
   */
  glm::vec3 &up() { return m_up; }
  /**
   * @brief Get the view matrix of camera
   *
   * @return matrix4&
   */
  glm::mat4 &view() { return m_view; }
  /**
   * @brief Get the projection matrix of camera
   *
   * @return matrix4&
   */
  glm::mat4 &projection() { return m_projection; }

private:
  // Position of camera
  glm::vec3 m_position;
  // Camera target vector
  glm::vec3 m_target;
  // Up vector
  glm::vec3 m_up;
  // view
  glm::mat4 m_view;
  // projection
  glm::mat4 m_projection;

  bool update_frustum = false;
  Frustum frustum;
};
} // namespace sinen
#endif
