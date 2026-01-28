#ifndef SINEN_CAMERA_HPP
#define SINEN_CAMERA_HPP

#include <math/geometry/bbox.hpp>
#include <math/geometry/ray.hpp>
#include <math/matrix.hpp>
#include <math/vector.hpp>


namespace sinen {
struct Frustum {
  Vec4 planes[6]; // x, y, z, w: ax + by + cz + d = 0
};

/**
 * @brief Camera class
 *
 */
class Camera {
public:
  static constexpr const char *metaTableName() { return "sn.Camera"; }
  /**
   * @brief Move to position, look at a target, and set up vector
   *
   * @param position
   * @param target
   * @param up
   */
  void lookat(const Vec3 &position, const Vec3 &target, const Vec3 &up);
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
  bool isAABBInFrustum(const AABB &aabb);

  /**
   * @brief Convert a screen-space position to a world-space ray.
   *
   * Screen-space coordinates are centered at (0, 0) with +Y up, and typically
   * match values from Mouse::getPosition().
   *
   * @param screenPos Screen-space position (center-origin).
   * @param viewportSize Viewport size in pixels.
   */
  Ray screenToWorldRay(const Vec2 &screenPos, const Vec2 &viewportSize) const;

  /**
   * @brief Convert a screen-space position to a world-space ray.
   *
   * Uses the current Window::size() as the viewport.
   *
   * @param screenPos Screen-space position (center-origin).
   */
  Ray screenToWorldRay(const Vec2 &screenPos) const;
  /**
   * @brief Get the position of camera
   *
   * @return Vec3&
   */
  Vec3 &getPosition() { return position; }
  /**
   * @brief Get the target of camera
   *
   * @return Vec3&
   */
  const Vec3 &getTarget() const { return target; }
  /**
   * @brief Get the up vector of camera
   *
   * @return Vec3&
   */
  const Vec3 &getUp() const { return up; }
  /**
   * @brief Get the view matrix of camera
   *
   * @return matrix4&
   */
  const Mat4 &getView() const { return view; }
  /**
   * @brief Get the projection matrix of camera
   *
   * @return matrix4&
   */
  const Mat4 &getProjection() const { return projection; }

private:
  // Position of camera
  Vec3 position;
  // Camera target vector
  Vec3 target;
  // Up vector
  Vec3 up;
  // view
  Mat4 view;
  // projection
  Mat4 projection;

  bool updateFrustum = false;
  Frustum frustum;
};
} // namespace sinen
#endif
