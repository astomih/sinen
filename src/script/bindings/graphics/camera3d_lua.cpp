#include <graphics/camera/camera3d.hpp>
#include <graphics/graphics.hpp>
#include <platform/window/window.hpp>
#include <script/luaapi.hpp>

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
  luaPushcfunction2(L, lCameraGetView);
  lua_setfield(L, -2, "getView");
  luaPushcfunction2(L, lCameraGetProjection);
  lua_setfield(L, -2, "getProjection");
  luaPushcfunction2(L, lCameraGetViewProjection);
  lua_setfield(L, -2, "getViewProjection");
  luaPushcfunction2(L, lCameraIsAabbInFrustum);
  lua_setfield(L, -2, "isAABBInFrustum");
  luaPushcfunction2(L, lCameraWorldToScreen);
  lua_setfield(L, -2, "worldToScreen");
  luaPushcfunction2(L, lCameraScreenToWorldRay);
  lua_setfield(L, -2, "screenToWorldRay");
  lua_pop(L, 1);

  pushSnNamed(L, "Camera3D");
  luaPushcfunction2(L, lCameraNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

} // namespace sinen
