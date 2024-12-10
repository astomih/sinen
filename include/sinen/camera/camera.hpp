#ifndef SINEN_CAMERA_HPP
#define SINEN_CAMERA_HPP
#include "../math/matrix4.hpp"
#include "../math/vector3.hpp"

namespace sinen {
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
  void lookat(const Vector3 &position, const Vector3 &target,
              const Vector3 &up);
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
   * @brief Get the position of camera
   *
   * @return vector3&
   */
  Vector3 &position() { return m_position; }
  /**
   * @brief Get the target of camera
   *
   * @return vector3&
   */
  Vector3 &target() { return m_target; }
  /**
   * @brief Get the up vector of camera
   *
   * @return vector3&
   */
  Vector3 &up() { return m_up; }
  /**
   * @brief Get the view matrix of camera
   *
   * @return matrix4&
   */
  matrix4 &view() { return m_view; }
  /**
   * @brief Get the projection matrix of camera
   *
   * @return matrix4&
   */
  matrix4 &projection() { return m_projection; }

private:
  // Position of camera
  Vector3 m_position;
  // Camera target vector
  Vector3 m_target;
  // Up vector
  Vector3 m_up;
  // view
  matrix4 m_view;
  // projection
  matrix4 m_projection;
};
} // namespace sinen
#endif
