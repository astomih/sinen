#include <graphics/camera/camera2d.hpp>
#include <platform/window/window.hpp>
#include <script/luaapi.hpp>

#include <math/vector.hpp>

namespace sinen {
Camera2D::Camera2D(const Vec2 &size) : rect_(Vec2(0, 0), size) {}
void Camera2D::resize(const Vec2 &size) {
  this->rect_.width = size.x;
  this->rect_.height = size.y;
}
Vec2 Camera2D::windowRatio() const { return Window::size() / rect_.size(); }
Vec2 Camera2D::invWindowRatio() const { return rect_.size() / Window::size(); }
Vec2 Camera2D::size() const { return rect_.size(); }
Vec2 Camera2D::half() const { return rect_.size() * 0.5f; }
Rect Camera2D::rect() const { return Rect(Vec2(0), size()); }
Vec2 Camera2D::topLeft() const { return rect().topLeft(); }
Vec2 Camera2D::topCenter() const { return rect().topCenter(); }
Vec2 Camera2D::topRight() const { return rect().topRight(); }
Vec2 Camera2D::left() const { return rect().left(); }
Vec2 Camera2D::center() const { return rect().center(); }
Vec2 Camera2D::right() const { return rect().right(); }
Vec2 Camera2D::bottomLeft() const { return rect().bottomLeft(); }
Vec2 Camera2D::bottomCenter() const { return rect().bottomCenter(); }
Vec2 Camera2D::bottomRight() const { return rect().bottomRight(); }

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

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Rect>(L, r.rect());
    return 1;
  });
  lua_setfield(L, -2, "rect");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.topLeft());
    return 1;
  });
  lua_setfield(L, -2, "topLeft");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.topCenter());
    return 1;
  });
  lua_setfield(L, -2, "topCenter");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.topRight());
    return 1;
  });
  lua_setfield(L, -2, "topRight");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.left());
    return 1;
  });
  lua_setfield(L, -2, "left");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.center());
    return 1;
  });
  lua_setfield(L, -2, "center");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.right());
    return 1;
  });
  lua_setfield(L, -2, "right");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.bottomLeft());
    return 1;
  });
  lua_setfield(L, -2, "bottomLeft");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.bottomCenter());
    return 1;
  });
  lua_setfield(L, -2, "bottomCenter");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Camera2D>(L, 1);
    udNewOwned<Vec2>(L, r.bottomRight());
    return 1;
  });
  lua_setfield(L, -2, "bottomRight");
  lua_pop(L, 1);

  pushSnNamed(L, "Camera2D");
  luaPushcfunction2(L, lCamera2DNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
