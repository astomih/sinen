#include "luaapi.hpp"
#include <graphics/camera/camera3d.hpp>
#include <graphics/graphics.hpp>
#include <platform/window/window.hpp>

namespace sinen {
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
  const auto &cam = udValue<Camera3D>(L, 1);
  udNewOwned<Vec3>(L, cam.getPosition());
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
static int lCameraGetView(lua_State *L) {
  const auto &cam = udValue<Camera3D>(L, 1);
  udNewOwned<Mat4>(L, cam.getView());
  return 1;
}
static int lCameraGetProjection(lua_State *L) {
  const auto &cam = udValue<Camera3D>(L, 1);
  udNewOwned<Mat4>(L, cam.getProjection());
  return 1;
}
static int lCameraGetViewProjection(lua_State *L) {
  const auto &cam = udValue<Camera3D>(L, 1);
  udNewOwned<Mat4>(L, cam.getView() * cam.getProjection());
  return 1;
}
static int lCameraIsAabbInFrustum(lua_State *L) {
  auto &cam = udValue<Camera3D>(L, 1);
  auto &aabb = udValue<AABB>(L, 2);
  lua_pushboolean(L, cam.isAABBInFrustum(aabb));
  return 1;
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
static int lCameraScreenToWorldRay(lua_State *L) {
  auto &cam = udValue<Camera3D>(L, 1);
  auto &screenPos = udValue<Vec2>(L, 2);

  udNewOwned<Ray>(L, cam.screenToWorldRay(screenPos));
  return 1;
}
void registerCamera(lua_State *L) {
  luaL_newmetatable(L, Camera3D::metaTableName());
  Binding::registerFunction(L, "__gc", udGc<Camera3D>);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "lookat", lCameraLookat);
  Binding::registerFunction(L, "perspective", lCameraPerspective);
  Binding::registerFunction(L, "orthographic", lCameraOrthographic);
  Binding::registerFunction(L, "getPosition", lCameraGetPosition);
  Binding::registerFunction(L, "getTarget", lCameraGetTarget);
  Binding::registerFunction(L, "getUp", lCameraGetUp);
  Binding::registerFunction(L, "getView", lCameraGetView);
  Binding::registerFunction(L, "getProjection", lCameraGetProjection);
  Binding::registerFunction(L, "getViewProjection", lCameraGetViewProjection);
  Binding::registerFunction(L, "isAABBInFrustum", lCameraIsAabbInFrustum);
  Binding::registerFunction(L, "worldToScreen", lCameraWorldToScreen);
  Binding::registerFunction(L, "screenToWorldRay", lCameraScreenToWorldRay);
  lua_pop(L, 1);

  pushSnNamed(L, "Camera3D");
  Binding::registerFunction(L, "new", lCameraNew);
  lua_pop(L, 1);
}

} // namespace sinen
