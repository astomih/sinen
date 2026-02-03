#include <graphics/camera/camera2d.hpp>
#include <platform/window/window.hpp>
#include <script/luaapi.hpp>

#include <math/vector.hpp>

namespace sinen {

Vec2 Camera2D::windowRatio() const { return Window::size() / cameraSize; }
Vec2 Camera2D::invWindowRatio() const { return cameraSize / Window::size(); }

static int lCamera2DNew(lua_State *L) {
  udNewOwned<Camera2D>(L, Camera2D{});
  return 1;
}
static int lCamera2DSize(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1);
  udNewOwned<Vec2>(L, cam.size());
  return 1;
}
static int lCamera2DHalf(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1);
  udNewOwned<Vec2>(L, cam.half());
  return 1;
}
static int lCamera2DResize(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1);
  auto &size = udValue<Vec2>(L, 2);
  cam.resize(size);
  return 0;
}
static int lCamera2DWindowRatio(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1);
  udNewOwned<Vec2>(L, cam.windowRatio());
  return 1;
}
static int lCamera2DInvWindowRatio(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1);
  udNewOwned<Vec2>(L, cam.invWindowRatio());
  return 1;
}
void registerCamera2D(lua_State *L) {
  luaL_newmetatable(L, Camera2D::metaTableName());
  luaPushcfunction2(L, udGc<Camera2D>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lCamera2DSize);
  lua_setfield(L, -2, "size");
  luaPushcfunction2(L, lCamera2DHalf);
  lua_setfield(L, -2, "half");
  luaPushcfunction2(L, lCamera2DResize);
  lua_setfield(L, -2, "resize");
  luaPushcfunction2(L, lCamera2DWindowRatio);
  lua_setfield(L, -2, "windowRatio");
  luaPushcfunction2(L, lCamera2DInvWindowRatio);
  lua_setfield(L, -2, "invWindowRatio");
  lua_pop(L, 1);

  pushSnNamed(L, "Camera2D");
  luaPushcfunction2(L, lCamera2DNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
