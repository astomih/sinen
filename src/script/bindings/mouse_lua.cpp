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
void registerMouse(lua_State *L) {
  pushSnNamed(L, "Mouse");
  luau::table(L, -1).raw_function("setRelative", lMouseSetRelative);
  luau::table(L, -1).function("isRelative", lMouseIsRelative);
  luau::table(L, -1).function("getPositionOnScene", lMouseGetPositionOnScene);
  luau::table(L, -1).function("getPosition", lMouseGetPosition);
  luau::table(L, -1).function("getDelta", lMouseGetDelta);
  luau::table(L, -1).raw_function("isPressed", lMouseIsPressed);
  luau::table(L, -1).raw_function("isReleased", lMouseIsReleased);
  luau::table(L, -1).raw_function("isDown", lMouseIsDown);
  luau::table(L, -1).function("setPosition", lMouseSetPosition);
  luau::table(L, -1).function("setPositionOnScene", lMouseSetPositionOnScene);
  luau::table(L, -1).function("getScrollWheel", lMouseGetScrollWheel);
  luau::table(L, -1).raw_function("hideCursor", lMouseHideCursor);
  luau::table(L, -1).value("LEFT", static_cast<int>(Mouse::LEFT));
  luau::table(L, -1).value("RIGHT", static_cast<int>(Mouse::RIGHT));
  luau::table(L, -1).value("MIDDLE", static_cast<int>(Mouse::MIDDLE));
  luau::table(L, -1).value("X1", static_cast<int>(Mouse::X1));
  luau::table(L, -1).value("X2", static_cast<int>(Mouse::X2));
  lua_pop(L, 1);
}
} // namespace sinen
