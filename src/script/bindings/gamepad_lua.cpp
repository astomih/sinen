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
static auto lGamepadGetLeftStick() { return GamePad::getLeftStick(); }
static auto lGamepadGetRightStick() { return GamePad::getRightStick(); }
static auto lGamepadIsConnected() { return GamePad::isConnected(); }
static int lGamepadGetName(lua_State *L) {
  lua_pushstring(L, GamePad::getName());
  return 1;
}
static auto lGamepadGetLeftTrigger() { return GamePad::getLeftTrigger(); }
static auto lGamepadGetRightTrigger() { return GamePad::getRightTrigger(); }
void registerGamepad(luau::table &sn) {
  auto gamepad = sn.get_or_create_table("Gamepad");
  gamepad.raw_function<lGamepadIsPressed>("isPressed");
  gamepad.raw_function<lGamepadIsReleased>("isReleased");
  gamepad.raw_function<lGamepadIsDown>("isDown");
  gamepad.function<lGamepadGetLeftStick>("getLeftStick");
  gamepad.function<lGamepadGetRightStick>("getRightStick");
  gamepad.function<lGamepadIsConnected>("isConnected");
  gamepad.raw_function<lGamepadGetName>("getName");
  gamepad.function<lGamepadGetLeftTrigger>("getLeftTrigger");
  gamepad.function<lGamepadGetRightTrigger>("getRightTrigger");

  auto gamepadButton = sn.get_or_create_table("GamepadButton");

  gamepadButton.value("INVALID", static_cast<int>(GamepadButton::INVALID));
  gamepadButton.value("A", static_cast<int>(GamepadButton::A));
  gamepadButton.value("B", static_cast<int>(GamepadButton::B));
  gamepadButton.value("X", static_cast<int>(GamepadButton::X));
  gamepadButton.value("Y", static_cast<int>(GamepadButton::Y));
  gamepadButton.value("BACK", static_cast<int>(GamepadButton::BACK));
  gamepadButton.value("GUIDE", static_cast<int>(GamepadButton::GUIDE));
  gamepadButton.value("START", static_cast<int>(GamepadButton::START));
  gamepadButton.value("LEFTSTICK", static_cast<int>(GamepadButton::LEFTSTICK));
  gamepadButton.value("RIGHTSTICK",
                      static_cast<int>(GamepadButton::RIGHTSTICK));
  gamepadButton.value("LEFTSHOULDER",
                      static_cast<int>(GamepadButton::LEFTSHOULDER));
  gamepadButton.value("RIGHTSHOULDER",
                      static_cast<int>(GamepadButton::RIGHTSHOULDER));
  gamepadButton.value("DPAD_UP", static_cast<int>(GamepadButton::DPAD_UP));
  gamepadButton.value("DPAD_DOWN", static_cast<int>(GamepadButton::DPAD_DOWN));
  gamepadButton.value("DPAD_LEFT", static_cast<int>(GamepadButton::DPAD_LEFT));
  gamepadButton.value("DPAD_RIGHT",
                      static_cast<int>(GamepadButton::DPAD_RIGHT));
  gamepadButton.value("MISC1", static_cast<int>(GamepadButton::MISC1));
  gamepadButton.value("PADDLE1", static_cast<int>(GamepadButton::PADDLE1));
  gamepadButton.value("PADDLE2", static_cast<int>(GamepadButton::PADDLE2));
  gamepadButton.value("PADDLE3", static_cast<int>(GamepadButton::PADDLE3));
  gamepadButton.value("PADDLE4", static_cast<int>(GamepadButton::PADDLE4));
  gamepadButton.value("TOUCHPAD", static_cast<int>(GamepadButton::TOUCHPAD));
}
} // namespace sinen
