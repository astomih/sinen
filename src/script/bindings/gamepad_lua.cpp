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
  Binding::registerFunction(L, "isPressed", lGamepadIsPressed);
  Binding::registerFunction(L, "isReleased", lGamepadIsReleased);
  Binding::registerFunction(L, "isDown", lGamepadIsDown);
  Binding::registerFunction(L, "getLeftStick", lGamepadGetLeftStick);
  Binding::registerFunction(L, "getRightStick", lGamepadGetRightStick);
  Binding::registerFunction(L, "isConnected", lGamepadIsConnected);
  lua_pop(L, 1);

  pushSnNamed(L, "GamepadButton");

  Binding::registerInteger(L, "INVALID",
                         static_cast<int>(GamepadButton::INVALID));
  Binding::registerInteger(L, "A", static_cast<int>(GamepadButton::A));
  Binding::registerInteger(L, "B", static_cast<int>(GamepadButton::B));
  Binding::registerInteger(L, "X", static_cast<int>(GamepadButton::X));
  Binding::registerInteger(L, "Y", static_cast<int>(GamepadButton::Y));
  Binding::registerInteger(L, "BACK", static_cast<int>(GamepadButton::BACK));
  Binding::registerInteger(L, "GUIDE", static_cast<int>(GamepadButton::GUIDE));
  Binding::registerInteger(L, "START", static_cast<int>(GamepadButton::START));
  Binding::registerInteger(L, "LEFTSTICK",
                         static_cast<int>(GamepadButton::LEFTSTICK));
  Binding::registerInteger(L, "RIGHTSTICK",
                         static_cast<int>(GamepadButton::RIGHTSTICK));
  Binding::registerInteger(L, "LEFTSHOULDER",
                         static_cast<int>(GamepadButton::LEFTSHOULDER));
  Binding::registerInteger(L, "RIGHTSHOULDER",
                         static_cast<int>(GamepadButton::RIGHTSHOULDER));
  Binding::registerInteger(L, "DPAD_UP",
                         static_cast<int>(GamepadButton::DPAD_UP));
  Binding::registerInteger(L, "DPAD_DOWN",
                         static_cast<int>(GamepadButton::DPAD_DOWN));
  Binding::registerInteger(L, "DPAD_LEFT",
                         static_cast<int>(GamepadButton::DPAD_LEFT));
  Binding::registerInteger(L, "DPAD_RIGHT",
                         static_cast<int>(GamepadButton::DPAD_RIGHT));
  Binding::registerInteger(L, "MISC1", static_cast<int>(GamepadButton::MISC1));
  Binding::registerInteger(L, "PADDLE1",
                         static_cast<int>(GamepadButton::PADDLE1));
  Binding::registerInteger(L, "PADDLE2",
                         static_cast<int>(GamepadButton::PADDLE2));
  Binding::registerInteger(L, "PADDLE3",
                         static_cast<int>(GamepadButton::PADDLE3));
  Binding::registerInteger(L, "PADDLE4",
                         static_cast<int>(GamepadButton::PADDLE4));
  Binding::registerInteger(L, "TOUCHPAD",
                         static_cast<int>(GamepadButton::TOUCHPAD));

  lua_pop(L, 1);
}
} // namespace sinen
