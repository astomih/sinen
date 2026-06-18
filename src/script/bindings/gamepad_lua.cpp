#include "luaapi.hpp"
#include <platform/input/gamepad.hpp>

namespace sinen {
static int lGamepadIsPressed(lua_State *L) {
  auto btn = static_cast<GamepadButton>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, GamePad::isPressed(btn));
  return 1;
}
static int lGamepadIsReleased(lua_State *L) {
  auto btn = static_cast<GamepadButton>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, GamePad::isReleased(btn));
  return 1;
}
static int lGamepadIsDown(lua_State *L) {
  auto btn = static_cast<GamepadButton>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, GamePad::isDown(btn));
  return 1;
}
static int lGamepadGetLeftStick(lua_State *L) {
  udNewOwned<Vec2>(L, GamePad::getLeftStick());
  return 1;
}
static int lGamepadGetRightStick(lua_State *L) {
  udNewOwned<Vec2>(L, GamePad::getRightStick());
  return 1;
}
static int lGamepadIsConnected(lua_State *L) {
  lua_pushboolean(L, GamePad::isConnected());
  return 1;
}
void registerGamepad(lua_State *L) {
  pushSnNamed(L, "Gamepad");
  luaPushcfunction2(L, lGamepadIsPressed);
  lua_setfield(L, -2, "isPressed");
  luaPushcfunction2(L, lGamepadIsReleased);
  lua_setfield(L, -2, "isReleased");
  luaPushcfunction2(L, lGamepadIsDown);
  lua_setfield(L, -2, "isDown");
  luaPushcfunction2(L, lGamepadGetLeftStick);
  lua_setfield(L, -2, "getLeftStick");
  luaPushcfunction2(L, lGamepadGetRightStick);
  lua_setfield(L, -2, "getRightStick");
  luaPushcfunction2(L, lGamepadIsConnected);
  lua_setfield(L, -2, "isConnected");
  lua_pop(L, 1);

  pushSnNamed(L, "GamepadButton");

  lua_pushinteger(L, static_cast<int>(GamepadButton::INVALID));
  lua_setfield(L, -2, "INVALID");
  lua_pushinteger(L, static_cast<int>(GamepadButton::A));
  lua_setfield(L, -2, "A");
  lua_pushinteger(L, static_cast<int>(GamepadButton::B));
  lua_setfield(L, -2, "B");
  lua_pushinteger(L, static_cast<int>(GamepadButton::X));
  lua_setfield(L, -2, "X");
  lua_pushinteger(L, static_cast<int>(GamepadButton::Y));
  lua_setfield(L, -2, "Y");
  lua_pushinteger(L, static_cast<int>(GamepadButton::BACK));
  lua_setfield(L, -2, "BACK");
  lua_pushinteger(L, static_cast<int>(GamepadButton::GUIDE));
  lua_setfield(L, -2, "GUIDE");
  lua_pushinteger(L, static_cast<int>(GamepadButton::START));
  lua_setfield(L, -2, "START");
  lua_pushinteger(L, static_cast<int>(GamepadButton::LEFTSTICK));
  lua_setfield(L, -2, "LEFTSTICK");
  lua_pushinteger(L, static_cast<int>(GamepadButton::RIGHTSTICK));
  lua_setfield(L, -2, "RIGHTSTICK");
  lua_pushinteger(L, static_cast<int>(GamepadButton::LEFTSHOULDER));
  lua_setfield(L, -2, "LEFTSHOULDER");
  lua_pushinteger(L, static_cast<int>(GamepadButton::RIGHTSHOULDER));
  lua_setfield(L, -2, "RIGHTSHOULDER");
  lua_pushinteger(L, static_cast<int>(GamepadButton::DPAD_UP));
  lua_setfield(L, -2, "DPAD_UP");
  lua_pushinteger(L, static_cast<int>(GamepadButton::DPAD_DOWN));
  lua_setfield(L, -2, "DPAD_DOWN");
  lua_pushinteger(L, static_cast<int>(GamepadButton::DPAD_LEFT));
  lua_setfield(L, -2, "DPAD_LEFT");
  lua_pushinteger(L, static_cast<int>(GamepadButton::DPAD_RIGHT));
  lua_setfield(L, -2, "DPAD_RIGHT");
  lua_pushinteger(L, static_cast<int>(GamepadButton::MISC1));
  lua_setfield(L, -2, "MISC1");
  lua_pushinteger(L, static_cast<int>(GamepadButton::PADDLE1));
  lua_setfield(L, -2, "PADDLE1");
  lua_pushinteger(L, static_cast<int>(GamepadButton::PADDLE2));
  lua_setfield(L, -2, "PADDLE2");
  lua_pushinteger(L, static_cast<int>(GamepadButton::PADDLE3));
  lua_setfield(L, -2, "PADDLE3");
  lua_pushinteger(L, static_cast<int>(GamepadButton::PADDLE4));
  lua_setfield(L, -2, "PADDLE4");
  lua_pushinteger(L, static_cast<int>(GamepadButton::TOUCHPAD));
  lua_setfield(L, -2, "TOUCHPAD");

  lua_pop(L, 1);
}
} // namespace sinen
