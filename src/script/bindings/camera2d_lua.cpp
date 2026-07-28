#include "luaapi.hpp"
#include <graphics/camera/camera2d.hpp>
#include <math/vector.hpp>
#include <platform/window/window.hpp>

namespace sinen {
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
  Binding::registerFunction(L, "__gc", udGc<Camera2D>);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "size", lCamera2DSize);
  Binding::registerFunction(L, "half", lCamera2DHalf);
  Binding::registerFunction(L, "resize", lCamera2DResize);
  Binding::registerFunction(L, "windowRatio", lCamera2DWindowRatio);
  Binding::registerFunction(L, "invWindowRatio", lCamera2DInvWindowRatio);

  Binding::registerFunction(L, "rect", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Rect>(L, r.rect());
    return 1;
  });

  Binding::registerFunction(L, "topLeft", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.topLeft());
    return 1;
  });

  Binding::registerFunction(L, "topCenter", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.topCenter());
    return 1;
  });

  Binding::registerFunction(L, "topRight", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.topRight());
    return 1;
  });

  Binding::registerFunction(L, "left", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.left());
    return 1;
  });

  Binding::registerFunction(L, "center", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.center());
    return 1;
  });

  Binding::registerFunction(L, "right", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.right());
    return 1;
  });

  Binding::registerFunction(L, "bottomLeft", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.bottomLeft());
    return 1;
  });

  Binding::registerFunction(L, "bottomCenter", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.bottomCenter());
    return 1;
  });

  Binding::registerFunction(L, "bottomRight", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.bottomRight());
    return 1;
  });
  lua_pop(L, 1);

  pushSnNamed(L, "Camera2D");
  Binding::registerFunction(L, "new", lCamera2DNew);
  lua_pop(L, 1);
}
} // namespace sinen
