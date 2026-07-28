#include "luaapi.hpp"
#include <platform/input/mouse.hpp>

namespace sinen {
static int lMouseGetPositionOnScene(lua_State *L) {
  udNewOwned<Vec2>(L, Mouse::getPositionOnScene());
  return 1;
}
static int lMouseGetPosition(lua_State *L) {
  udNewOwned<Vec2>(L, Mouse::getPosition());
  return 1;
}
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
static int lMouseSetPosition(lua_State *L) {
  auto &p = udValue<Vec2>(L, 1);
  Mouse::setPosition(p);
  return 0;
}
static int lMouseSetPositionOnScene(lua_State *L) {
  auto &p = udValue<Vec2>(L, 1);
  Mouse::setPositionOnScene(p);
  return 0;
}
static int lMouseGetScrollWheel(lua_State *L) {
  udNewOwned<Vec2>(L, Mouse::getScrollWheel());
  return 1;
}
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
static int lMouseIsRelative(lua_State *L) {
  lua_pushboolean(L, Mouse::isRelative());
  return 1;
}
void registerMouse(lua_State *L) {
  pushSnNamed(L, "Mouse");
  Binding::registerFunction(L, "setRelative", lMouseSetRelative);
  Binding::registerFunction(L, "isRelative", lMouseIsRelative);
  Binding::registerFunction(L, "getPositionOnScene", lMouseGetPositionOnScene);
  Binding::registerFunction(L, "getPosition", lMouseGetPosition);
  Binding::registerFunction(L, "isPressed", lMouseIsPressed);
  Binding::registerFunction(L, "isReleased", lMouseIsReleased);
  Binding::registerFunction(L, "isDown", lMouseIsDown);
  Binding::registerFunction(L, "setPosition", lMouseSetPosition);
  Binding::registerFunction(L, "setPositionOnScene", lMouseSetPositionOnScene);
  Binding::registerFunction(L, "getScrollWheel", lMouseGetScrollWheel);
  Binding::registerFunction(L, "hideCursor", lMouseHideCursor);
  Binding::registerInteger(L, "LEFT", static_cast<int>(Mouse::LEFT));
  Binding::registerInteger(L, "RIGHT", static_cast<int>(Mouse::RIGHT));
  Binding::registerInteger(L, "MIDDLE", static_cast<int>(Mouse::MIDDLE));
  Binding::registerInteger(L, "X1", static_cast<int>(Mouse::X1));
  Binding::registerInteger(L, "X2", static_cast<int>(Mouse::X2));
  lua_pop(L, 1);
}
} // namespace sinen
