#include "luaapi.hpp"
#include <graphics/camera/camera2d.hpp>
#include <math/vector.hpp>
#include <platform/window/window.hpp>

namespace sinen {
static auto lCamera2DNew() { return Camera2D{}; }
static auto lCamera2DSize(Camera2D &cam) { return cam.size(); }
static auto lCamera2DHalf(Camera2D &cam) { return cam.half(); }
static auto lCamera2DResize(Camera2D &cam, Vec2 &size) {
  cam.resize(size);
  return;
}
static auto lCamera2DWindowRatio(Camera2D &cam) { return cam.windowRatio(); }
static auto lCamera2DInvWindowRatio(Camera2D &cam) {
  return cam.invWindowRatio();
}
void registerCamera2D(lua_State *L) {
  pushClassMetatable<Camera2D>(L, Camera2D::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luau::table(L, -1).function("size", lCamera2DSize);
  luau::table(L, -1).function("half", lCamera2DHalf);
  luau::table(L, -1).function("resize", lCamera2DResize);
  luau::table(L, -1).function("windowRatio", lCamera2DWindowRatio);
  luau::table(L, -1).function("invWindowRatio", lCamera2DInvWindowRatio);

  luau::table(L, -1).raw_function("rect", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Rect>(L, r.rect());
    return 1;
  });

  luau::table(L, -1).raw_function("topLeft", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.topLeft());
    return 1;
  });

  luau::table(L, -1).raw_function("topCenter", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.topCenter());
    return 1;
  });

  luau::table(L, -1).raw_function("topRight", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.topRight());
    return 1;
  });

  luau::table(L, -1).raw_function("left", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.left());
    return 1;
  });

  luau::table(L, -1).raw_function("center", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.center());
    return 1;
  });

  luau::table(L, -1).raw_function("right", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.right());
    return 1;
  });

  luau::table(L, -1).raw_function("bottomLeft", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.bottomLeft());
    return 1;
  });

  luau::table(L, -1).raw_function("bottomCenter", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.bottomCenter());
    return 1;
  });

  luau::table(L, -1).raw_function("bottomRight", [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.bottomRight());
    return 1;
  });
  lua_pop(L, 1);

  pushSnNamed(L, "Camera2D");
  luau::table(L, -1).function("new", lCamera2DNew);
  lua_pop(L, 1);
}
} // namespace sinen
