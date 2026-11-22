#include "luaapi.hpp"
#include <platform/input/mouse.hpp>

namespace sinen {
static auto lMouseGetPositionOnScene() { return Mouse::getPositionOnScene(); }
static auto lMouseGetPosition() { return Mouse::getPosition(); }
static auto lMouseGetDelta() { return Mouse::getDelta(); }
static int lMouseIsPressed(lua_State *L) {
  auto b = static_cast<Mouse::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Mouse::isPressed(b));
  return 1;
}
static int lMouseIsReleased(lua_State *L) {
  auto b = static_cast<Mouse::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Mouse::isReleased(b));
  return 1;
}
static int lMouseIsDown(lua_State *L) {
  auto b = static_cast<Mouse::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Mouse::isDown(b));
  return 1;
}
static auto lMouseSetPosition(Vec2 &p) {
  Mouse::setPosition(p);
  return;
}
static auto lMouseSetPositionOnScene(Vec2 &p) {
  Mouse::setPositionOnScene(p);
  return;
}
static auto lMouseGetScrollWheel() { return Mouse::getScrollWheel(); }
static int lMouseHideCursor(lua_State *L) {
  bool hide = lua_toboolean(L, 1) != 0;
  Mouse::hideCursor(hide);
  return 0;
}
static int lMouseSetRelative(lua_State *L) {
  bool rel = lua_toboolean(L, 1) != 0;
  Mouse::setRelative(rel);
  return 0;
}
static auto lMouseIsRelative() { return Mouse::isRelative(); }
void registerMouse(luau::table &sn) {
  auto mouse = sn.get_or_create_table("Mouse");
  mouse.raw_function<lMouseSetRelative>("setRelative");
  mouse.function<lMouseIsRelative>("isRelative");
  mouse.function<lMouseGetPositionOnScene>("getPositionOnScene");
  mouse.function<lMouseGetPosition>("getPosition");
  mouse.function<lMouseGetDelta>("getDelta");
  mouse.raw_function<lMouseIsPressed>("isPressed");
  mouse.raw_function<lMouseIsReleased>("isReleased");
  mouse.raw_function<lMouseIsDown>("isDown");
  mouse.function<lMouseSetPosition>("setPosition");
  mouse.function<lMouseSetPositionOnScene>("setPositionOnScene");
  mouse.function<lMouseGetScrollWheel>("getScrollWheel");
  mouse.raw_function<lMouseHideCursor>("hideCursor");
  mouse.value("LEFT", static_cast<int>(Mouse::LEFT));
  mouse.value("RIGHT", static_cast<int>(Mouse::RIGHT));
  mouse.value("MIDDLE", static_cast<int>(Mouse::MIDDLE));
  mouse.value("X1", static_cast<int>(Mouse::X1));
  mouse.value("X2", static_cast<int>(Mouse::X2));
}
} // namespace sinen
