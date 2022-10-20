#ifndef SINEN_CAMERA_HPP
#define SINEN_CAMERA_HPP
#include "../math/matrix4.hpp"
#include "../math/vector3.hpp"

namespace sinen {
/**
 * @brief Camera class
 *
 */
class camera {
public:
  /**
   * @brief Move to position, look at a target, and set up vector
   *
   * @param position
   * @param target
   * @param up
   */
  static void lookat(const vector3 &position, const vector3 &target,
                     const vector3 &up);
  /**
   * @brief Set Perspective projection
   *
   * @param fov Field of view
   * @param aspect Aspect ratio
   * @param near Near plane
   * @param far Far plane
   */
  static void perspective(float fov, float aspect, float near, float far);
  /**
   * @brief Set Orthographic projection
   *
   * @param width Width of the orthographic projection
   * @param height Height of the orthographic projection
   * @param near Near plane
   * @param far Far plane
   */
  static void orthographic(float width, float height, float near, float far);
  /**
   * @brief Get the position of camera
   *
   * @return const vector3&
   */
  const static vector3 &position() { return m_position; }
  /**
   * @brief Get the target of camera
   *
   * @return const vector3&
   */
  const static vector3 &target() { return m_target; }
  /**
   * @brief Get the up vector of camera
   *
   * @return const vector3&
   */
  const static vector3 &up() { return m_up; }
  /**
   * @brief Get the view matrix of camera
   *
   * @return const matrix4&
   */
  const static matrix4 &view() { return m_view; }
  /**
   * @brief Get the projection matrix of camera
   *
   * @return const matrix4&
   */
  const static matrix4 &projection() { return m_projection; }

private:
  // Position of camera
  static vector3 m_position;
  // Camera target vector
  static vector3 m_target;
  // Up vector
  static vector3 m_up;
  // view
  static matrix4 m_view;
  // projection
  static matrix4 m_projection;
};
} // namespace sinen
#endif
