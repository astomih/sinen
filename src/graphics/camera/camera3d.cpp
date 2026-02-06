// internal
#include <graphics/camera/camera3d.hpp>
#include <graphics/graphics.hpp>
#include <platform/window/window.hpp>
#include <script/luaapi.hpp>

namespace sinen {
static Vec3 perspDiv(const Vec4 &v) {
  if (v.w == 0.0f) {
    return Vec3(v.x, v.y, v.z);
  }
  return Vec3(v.x / v.w, v.y / v.w, v.z / v.w);
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

static int lCameraNew(lua_State *L) {
  udNewOwned<Camera3D>(L, Camera3D{});
  return 1;
}
static int lCameraLookat(lua_State *L) {
  auto &cam = udValue<Camera3D>(L, 1);
  auto &pos = udValue<Vec3>(L, 2);
  auto &target = udValue<Vec3>(L, 3);
  auto &up = udValue<Vec3>(L, 4);
  cam.lookat(pos, target, up);
  return 0;
}
static int lCameraPerspective(lua_State *L) {
  auto &cam = udValue<Camera3D>(L, 1);
  float fov = static_cast<float>(luaL_checknumber(L, 2));
  float aspect = static_cast<float>(luaL_checknumber(L, 3));
  float nearZ = static_cast<float>(luaL_checknumber(L, 4));
  float farZ = static_cast<float>(luaL_checknumber(L, 5));
  cam.perspective(fov, aspect, nearZ, farZ);
  return 0;
}
static int lCameraOrthographic(lua_State *L) {
  auto &cam = udValue<Camera3D>(L, 1);
  float w = static_cast<float>(luaL_checknumber(L, 2));
  float h = static_cast<float>(luaL_checknumber(L, 3));
  float nearZ = static_cast<float>(luaL_checknumber(L, 4));
  float farZ = static_cast<float>(luaL_checknumber(L, 5));
  cam.orthographic(w, h, nearZ, farZ);
  return 0;
}
static int lCameraGetPosition(lua_State *L) {
  auto &cam = udValue<Camera3D>(L, 1);
  udNewRef<Vec3>(L, &cam.getPosition());
  return 1;
}
static int lCameraGetTarget(lua_State *L) {
  auto &cam = udValue<Camera3D>(L, 1);
  udNewOwned<Vec3>(L, cam.getTarget());
  return 1;
}
static int lCameraGetUp(lua_State *L) {
  auto &cam = udValue<Camera3D>(L, 1);
  udNewOwned<Vec3>(L, cam.getUp());
  return 1;
}
static int lCameraIsAabbInFrustum(lua_State *L) {
  auto &cam = udValue<Camera3D>(L, 1);
  auto &aabb = udValue<AABB>(L, 2);
  lua_pushboolean(L, cam.isAABBInFrustum(aabb));
  return 1;
}
static int lCameraScreenToWorldRay(lua_State *L) {
  auto &cam = udValue<Camera3D>(L, 1);
  auto &screenPos = udValue<Vec2>(L, 2);

  udNewOwned<Ray>(L, cam.screenToWorldRay(screenPos));
  return 1;
}
void registerCamera(lua_State *L) {
  luaL_newmetatable(L, Camera3D::metaTableName());
  luaPushcfunction2(L, udGc<Camera3D>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lCameraLookat);
  lua_setfield(L, -2, "lookat");
  luaPushcfunction2(L, lCameraPerspective);
  lua_setfield(L, -2, "perspective");
  luaPushcfunction2(L, lCameraOrthographic);
  lua_setfield(L, -2, "orthographic");
  luaPushcfunction2(L, lCameraGetPosition);
  lua_setfield(L, -2, "getPosition");
  luaPushcfunction2(L, lCameraGetTarget);
  lua_setfield(L, -2, "getTarget");
  luaPushcfunction2(L, lCameraGetUp);
  lua_setfield(L, -2, "getUp");
  luaPushcfunction2(L, lCameraIsAabbInFrustum);
  lua_setfield(L, -2, "isAABBInFrustum");
  luaPushcfunction2(L, lCameraScreenToWorldRay);
  lua_setfield(L, -2, "screenToWorldRay");
  lua_pop(L, 1);

  pushSnNamed(L, "Camera3D");
  luaPushcfunction2(L, lCameraNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

} // namespace sinen
