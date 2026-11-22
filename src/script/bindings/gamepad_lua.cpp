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
void registerGamepad(lua_State *L) {
  pushSnNamed(L, "Gamepad");
  luau::table(L, -1).raw_function("isPressed", lGamepadIsPressed);
  luau::table(L, -1).raw_function("isReleased", lGamepadIsReleased);
  luau::table(L, -1).raw_function("isDown", lGamepadIsDown);
  luau::table(L, -1).function("getLeftStick", lGamepadGetLeftStick);
  luau::table(L, -1).function("getRightStick", lGamepadGetRightStick);
  luau::table(L, -1).function("isConnected", lGamepadIsConnected);
  luau::table(L, -1).raw_function("getName", lGamepadGetName);
  luau::table(L, -1).function("getLeftTrigger", lGamepadGetLeftTrigger);
  luau::table(L, -1).function("getRightTrigger", lGamepadGetRightTrigger);
  lua_pop(L, 1);

  pushSnNamed(L, "GamepadButton");

  luau::table(L, -1).value("INVALID", static_cast<int>(GamepadButton::INVALID));
  luau::table(L, -1).value("A", static_cast<int>(GamepadButton::A));
  luau::table(L, -1).value("B", static_cast<int>(GamepadButton::B));
  luau::table(L, -1).value("X", static_cast<int>(GamepadButton::X));
  luau::table(L, -1).value("Y", static_cast<int>(GamepadButton::Y));
  luau::table(L, -1).value("BACK", static_cast<int>(GamepadButton::BACK));
  luau::table(L, -1).value("GUIDE", static_cast<int>(GamepadButton::GUIDE));
  luau::table(L, -1).value("START", static_cast<int>(GamepadButton::START));
  luau::table(L, -1).value("LEFTSTICK",
                           static_cast<int>(GamepadButton::LEFTSTICK));
  luau::table(L, -1).value("RIGHTSTICK",
                           static_cast<int>(GamepadButton::RIGHTSTICK));
  luau::table(L, -1).value("LEFTSHOULDER",
                           static_cast<int>(GamepadButton::LEFTSHOULDER));
  luau::table(L, -1).value("RIGHTSHOULDER",
                           static_cast<int>(GamepadButton::RIGHTSHOULDER));
  luau::table(L, -1).value("DPAD_UP", static_cast<int>(GamepadButton::DPAD_UP));
  luau::table(L, -1).value("DPAD_DOWN",
                           static_cast<int>(GamepadButton::DPAD_DOWN));
  luau::table(L, -1).value("DPAD_LEFT",
                           static_cast<int>(GamepadButton::DPAD_LEFT));
  luau::table(L, -1).value("DPAD_RIGHT",
                           static_cast<int>(GamepadButton::DPAD_RIGHT));
  luau::table(L, -1).value("MISC1", static_cast<int>(GamepadButton::MISC1));
  luau::table(L, -1).value("PADDLE1", static_cast<int>(GamepadButton::PADDLE1));
  luau::table(L, -1).value("PADDLE2", static_cast<int>(GamepadButton::PADDLE2));
  luau::table(L, -1).value("PADDLE3", static_cast<int>(GamepadButton::PADDLE3));
  luau::table(L, -1).value("PADDLE4", static_cast<int>(GamepadButton::PADDLE4));
  luau::table(L, -1).value("TOUCHPAD",
                           static_cast<int>(GamepadButton::TOUCHPAD));

  lua_pop(L, 1);
}
} // namespace sinen
