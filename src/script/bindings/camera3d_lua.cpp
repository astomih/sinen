#include "luaapi.hpp"
#include <graphics/camera/camera3d.hpp>
#include <graphics/graphics.hpp>
#include <platform/window/window.hpp>

namespace sinen {
static auto lCameraNew() { return Camera3D{}; }
static auto lCameraLookat(Camera3D &cam, Vec3 &pos, Vec3 &target, Vec3 &up) {
  cam.lookat(pos, target, up);
  return;
}
static auto lCameraPerspective(Camera3D &cam, float fov, float aspect,
                               float nearZ, float farZ) {
  cam.perspective(fov, aspect, nearZ, farZ);
  return;
}
static auto lCameraOrthographic(Camera3D &cam, float w, float h, float nearZ,
                                float farZ) {
  cam.orthographic(w, h, nearZ, farZ);
  return;
}
static auto lCameraGetPosition(const Camera3D &cam) {
  return cam.getPosition();
}
static auto lCameraGetTarget(Camera3D &cam) { return cam.getTarget(); }
static auto lCameraGetUp(Camera3D &cam) { return cam.getUp(); }
static auto lCameraGetView(const Camera3D &cam) { return cam.getView(); }
static auto lCameraGetProjection(const Camera3D &cam) {
  return cam.getProjection();
}
static auto lCameraGetViewProjection(const Camera3D &cam) {
  return cam.getView() * cam.getProjection();
}
static auto lCameraIsAabbInFrustum(Camera3D &cam, AABB &aabb) {
  return cam.isAABBInFrustum(aabb);
}
static int lCameraWorldToScreen(lua_State *L) {
  auto &cam = udValue<Camera3D>(L, 1);
  auto &worldPosition = udValue<Vec3>(L, 2);
  if (lua_gettop(L) >= 3) {
    auto &viewportSize = udValue<Vec2>(L, 3);
    udNewOwned<Vec3>(L, cam.worldToScreen(worldPosition, viewportSize));
    return 1;
  }
  udNewOwned<Vec3>(L, cam.worldToScreen(worldPosition));
  return 1;
}
static auto lCameraScreenToWorldRay(Camera3D &cam, Vec2 &screenPos) {
  return cam.screenToWorldRay(screenPos);
}
void registerCamera(lua_State *L) {
  pushClassMetatable<Camera3D>(L, Camera3D::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luau::table(L, -1).function("lookat", lCameraLookat);
  luau::table(L, -1).function("perspective", lCameraPerspective);
  luau::table(L, -1).function("orthographic", lCameraOrthographic);
  luau::table(L, -1).function("getPosition", lCameraGetPosition);
  luau::table(L, -1).function("getTarget", lCameraGetTarget);
  luau::table(L, -1).function("getUp", lCameraGetUp);
  luau::table(L, -1).function("getView", lCameraGetView);
  luau::table(L, -1).function("getProjection", lCameraGetProjection);
  luau::table(L, -1).function("getViewProjection", lCameraGetViewProjection);
  luau::table(L, -1).function("isAABBInFrustum", lCameraIsAabbInFrustum);
  luau::table(L, -1).raw_function("worldToScreen", lCameraWorldToScreen);
  luau::table(L, -1).function("screenToWorldRay", lCameraScreenToWorldRay);
  lua_pop(L, 1);

  pushSnNamed(L, "Camera3D");
  luau::table(L, -1).function("new", lCameraNew);
  lua_pop(L, 1);
}

} // namespace sinen
