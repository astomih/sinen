// internal
#include <graphics/camera/camera3d.hpp>
#include <graphics/graphics.hpp>
#include <platform/window/window.hpp>

namespace sinen {
static Vec3 perspDiv(const Vec4 &v) {
  if (v.w == 0.0f) {
    return Vec3(v.x, v.y, v.z);
  }
  return Vec3(v.x / v.w, v.y / v.w, v.z / v.w);
}

Camera3D::Camera3D() {
  lookat(Vec3{0, -1, 1}, Vec3{0, 0, 0}, Vec3{0, 0, 1});
  perspective(90.f, Window::size().x / Window::size().y, .1f, 100.f);
}

void Camera3D::lookat(const Vec3 &position, const Vec3 &target,
                      const Vec3 &up) {
  this->position = position;
  this->target = target;
  this->up = up;
  this->view = Mat4::lookat(position, target, up);
  view = (view);
  this->updateFrustum = true;
}
void Camera3D::perspective(float fov, float aspect, float near, float far) {
  this->projection = Mat4::perspective(Math::toRadians(fov), aspect, near, far);
  projection = (projection);
  this->updateFrustum = true;
}
void Camera3D::orthographic(float width, float height, float near, float far) {
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
bool Camera3D::isAABBInFrustum(const AABB &aabb) {
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

Vec3 Camera3D::worldToScreen(const Vec3 &worldPosition,
                             const Vec2 &viewportSize) const {
  const Mat4 vp = this->projection * this->view;
  const Vec4 world(worldPosition, 1.0f);
  const Vec4 clip(vp[0][0] * world.x + vp[0][1] * world.y + vp[0][2] * world.z +
                      vp[0][3] * world.w,
                  vp[1][0] * world.x + vp[1][1] * world.y + vp[1][2] * world.z +
                      vp[1][3] * world.w,
                  vp[2][0] * world.x + vp[2][1] * world.y + vp[2][2] * world.z +
                      vp[2][3] * world.w,
                  vp[3][0] * world.x + vp[3][1] * world.y + vp[3][2] * world.z +
                      vp[3][3] * world.w);
  const Vec3 ndc = perspDiv(clip);
  return Vec3((ndc.x * 0.5f + 0.5f) * viewportSize.x,
              (1.0f - (ndc.y * 0.5f + 0.5f)) * viewportSize.y, ndc.z);
}

Vec3 Camera3D::worldToScreen(const Vec3 &worldPosition) const {
  return worldToScreen(worldPosition, Window::size());
}

Ray Camera3D::screenToWorldRay(const Vec2 &screenPos,
                               const Vec2 &viewportSize) const {
  const Vec2 half = viewportSize * 0.5f;
  const float ndcX = screenPos.x / half.x;
  const float ndcY = screenPos.y / half.y;

  const Mat4 invVP = Mat4::invert(this->projection * this->view);

  const Vec4 nearClip(ndcX, ndcY, 0.0f, 1.0f);
  const Vec4 farClip(ndcX, ndcY, 1.0f, 1.0f);

  const Vec3 nearWorld = perspDiv(invVP * nearClip);
  const Vec3 farWorld = perspDiv(invVP * farClip);

  Ray ray;
  ray.origin = nearWorld;
  ray.direction = Vec3::normalize(farWorld - nearWorld);
  return ray;
}

Ray Camera3D::screenToWorldRay(const Vec2 &screenPos) const {
  return screenToWorldRay(screenPos, Window::size());
}

} // namespace sinen
