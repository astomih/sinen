// internal
#include <graphics/camera/camera.hpp>
#include <graphics/graphics.hpp>
#include <platform/window/window.hpp>

namespace sinen {
void Camera::lookat(const Vec3 &position, const Vec3 &target, const Vec3 &up) {
  this->position = position;
  this->target = target;
  this->up = up;
  this->view = Mat4::lookat(position, target, up);
  view = (view);
  this->updateFrustum = true;
}
void Camera::perspective(float fov, float aspect, float near, float far) {
  this->projection = Mat4::perspective(Math::toRadians(fov), aspect, near, far);
  projection = (projection);
  this->updateFrustum = true;
}
void Camera::orthographic(float width, float height, float near, float far) {
  this->projection = Mat4::ortho(width, height, near, far);
  this->updateFrustum = true;
}

static Frustum extractFrustumPlanes(const Mat4 &vp) {
  Frustum f;

  // Left   : row3 + row0
  f.planes[0] = Vec4(vp[3][0] + vp[0][0], vp[3][1] + vp[0][1],
                     vp[3][2] + vp[0][2], vp[3][3] + vp[0][3]);

  // Right  : row3 - row0
  f.planes[1] = Vec4(vp[3][0] - vp[0][0], vp[3][1] - vp[0][1],
                     vp[3][2] - vp[0][2], vp[3][3] - vp[0][3]);

  // Top    : row3 - row1
  f.planes[2] = Vec4(vp[3][0] - vp[1][0], vp[3][1] - vp[1][1],
                     vp[3][2] - vp[1][2], vp[3][3] - vp[1][3]);

  // Bottom : row3 + row1
  f.planes[3] = Vec4(vp[3][0] + vp[1][0], vp[3][1] + vp[1][1],
                     vp[3][2] + vp[1][2], vp[3][3] + vp[1][3]);

  // Near   : row3 + row2
  f.planes[4] = Vec4(vp[3][0] + vp[2][0], vp[3][1] + vp[2][1],
                     vp[3][2] + vp[2][2], vp[3][3] + vp[2][3]);

  // Far    : row3 - row2
  f.planes[5] = Vec4(vp[3][0] - vp[2][0], vp[3][1] - vp[2][1],
                     vp[3][2] - vp[2][2], vp[3][3] - vp[2][3]);

  // Normalize
  for (int i = 0; i < 6; i++) {
    float length = Vec3(f.planes[i]).length();
    f.planes[i] /= length;
  }

  return f;
}
bool Camera::isAABBInFrustum(const AABB &aabb) {
  if (this->updateFrustum) {
    this->frustum = extractFrustumPlanes(this->projection * this->view);
    this->updateFrustum = false;
  }
  for (int i = 0; i < 6; ++i) {
    const Vec4 &plane = frustum.planes[i];
    Vec3 normal = Vec3(plane);
    float d = plane.w;

    Vec3 p = Vec3((normal.x > 0 ? aabb.max.x : aabb.min.x),
                  (normal.y > 0 ? aabb.max.y : aabb.min.y),
                  (normal.z > 0 ? aabb.max.z : aabb.min.z));

    if (Vec3::dot(normal, p) + d < 0) {
      return false;
    }
  }

  return true;
}

} // namespace sinen
