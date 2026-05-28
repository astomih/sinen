#include <platform/input/mouse.hpp>
#include <script/luaapi.hpp>

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
  luaPushcfunction2(L, lMouseSetRelative);
  lua_setfield(L, -2, "setRelative");
  luaPushcfunction2(L, lMouseIsRelative);
  lua_setfield(L, -2, "isRelative");
  luaPushcfunction2(L, lMouseGetPositionOnScene);
  lua_setfield(L, -2, "getPositionOnScene");
  luaPushcfunction2(L, lMouseGetPosition);
  lua_setfield(L, -2, "getPosition");
  luaPushcfunction2(L, lMouseIsPressed);
  lua_setfield(L, -2, "isPressed");
  luaPushcfunction2(L, lMouseIsReleased);
  lua_setfield(L, -2, "isReleased");
  luaPushcfunction2(L, lMouseIsDown);
  lua_setfield(L, -2, "isDown");
  luaPushcfunction2(L, lMouseSetPosition);
  lua_setfield(L, -2, "setPosition");
  luaPushcfunction2(L, lMouseSetPositionOnScene);
  lua_setfield(L, -2, "setPositionOnScene");
  luaPushcfunction2(L, lMouseGetScrollWheel);
  lua_setfield(L, -2, "getScrollWheel");
  luaPushcfunction2(L, lMouseHideCursor);
  lua_setfield(L, -2, "hideCursor");
  lua_pushinteger(L, static_cast<int>(Mouse::LEFT));
  lua_setfield(L, -2, "LEFT");
  lua_pushinteger(L, static_cast<int>(Mouse::RIGHT));
  lua_setfield(L, -2, "RIGHT");
  lua_pushinteger(L, static_cast<int>(Mouse::MIDDLE));
  lua_setfield(L, -2, "MIDDLE");
  lua_pushinteger(L, static_cast<int>(Mouse::X1));
  lua_setfield(L, -2, "X1");
  lua_pushinteger(L, static_cast<int>(Mouse::X2));
  lua_setfield(L, -2, "X2");
  lua_pop(L, 1);
}
} // namespace sinen
