#include "luaapi.hpp"
#include <platform/input/keyboard.hpp>

namespace sinen {
static int lKeyboardIsPressed(lua_State *L) {
  auto key = static_cast<Scancode>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Keyboard::isPressed(key));
  return 1;
}
static int lKeyboardIsReleased(lua_State *L) {
  auto key = static_cast<Scancode>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Keyboard::isReleased(key));
  return 1;
}
static int lKeyboardIsDown(lua_State *L) {
  auto key = static_cast<Scancode>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Keyboard::isDown(key));
  return 1;
}
void registerKeyboard(lua_State *L) {
  pushSnNamed(L, "Keyboard");
  Binding::registerFunction(L, "isPressed", lKeyboardIsPressed);
  Binding::registerFunction(L, "isReleased", lKeyboardIsReleased);
  Binding::registerFunction(L, "isDown", lKeyboardIsDown);
  lua_pop(L, 1);

  pushSnNamed(L, "Scancode");

  Binding::registerInteger(L, "A", static_cast<int>(Scancode::A));
  Binding::registerInteger(L, "B", static_cast<int>(Scancode::B));
  Binding::registerInteger(L, "C", static_cast<int>(Scancode::C));
  Binding::registerInteger(L, "D", static_cast<int>(Scancode::D));
  Binding::registerInteger(L, "E", static_cast<int>(Scancode::E));
  Binding::registerInteger(L, "F", static_cast<int>(Scancode::F));
  Binding::registerInteger(L, "G", static_cast<int>(Scancode::G));
  Binding::registerInteger(L, "H", static_cast<int>(Scancode::H));
  Binding::registerInteger(L, "I", static_cast<int>(Scancode::I));
  Binding::registerInteger(L, "J", static_cast<int>(Scancode::J));
  Binding::registerInteger(L, "K", static_cast<int>(Scancode::K));
  Binding::registerInteger(L, "L", static_cast<int>(Scancode::L));
  Binding::registerInteger(L, "M", static_cast<int>(Scancode::M));
  Binding::registerInteger(L, "N", static_cast<int>(Scancode::N));
  Binding::registerInteger(L, "O", static_cast<int>(Scancode::O));
  Binding::registerInteger(L, "P", static_cast<int>(Scancode::P));
  Binding::registerInteger(L, "Q", static_cast<int>(Scancode::Q));
  Binding::registerInteger(L, "R", static_cast<int>(Scancode::R));
  Binding::registerInteger(L, "S", static_cast<int>(Scancode::S));
  Binding::registerInteger(L, "T", static_cast<int>(Scancode::T));
  Binding::registerInteger(L, "U", static_cast<int>(Scancode::U));
  Binding::registerInteger(L, "V", static_cast<int>(Scancode::V));
  Binding::registerInteger(L, "W", static_cast<int>(Scancode::W));
  Binding::registerInteger(L, "X", static_cast<int>(Scancode::X));
  Binding::registerInteger(L, "Y", static_cast<int>(Scancode::Y));
  Binding::registerInteger(L, "Z", static_cast<int>(Scancode::Z));

  Binding::registerInteger(L, "key0", static_cast<int>(Scancode::Key0));
  Binding::registerInteger(L, "key1", static_cast<int>(Scancode::Key1));
  Binding::registerInteger(L, "key2", static_cast<int>(Scancode::Key2));
  Binding::registerInteger(L, "key3", static_cast<int>(Scancode::Key3));
  Binding::registerInteger(L, "key4", static_cast<int>(Scancode::Key4));
  Binding::registerInteger(L, "key5", static_cast<int>(Scancode::Key5));
  Binding::registerInteger(L, "key6", static_cast<int>(Scancode::Key6));
  Binding::registerInteger(L, "key7", static_cast<int>(Scancode::Key7));
  Binding::registerInteger(L, "key8", static_cast<int>(Scancode::Key8));
  Binding::registerInteger(L, "key9", static_cast<int>(Scancode::Key9));

  Binding::registerInteger(L, "F1", static_cast<int>(Scancode::F1));
  Binding::registerInteger(L, "F2", static_cast<int>(Scancode::F2));
  Binding::registerInteger(L, "F3", static_cast<int>(Scancode::F3));
  Binding::registerInteger(L, "F4", static_cast<int>(Scancode::F4));
  Binding::registerInteger(L, "F5", static_cast<int>(Scancode::F5));
  Binding::registerInteger(L, "F6", static_cast<int>(Scancode::F6));
  Binding::registerInteger(L, "F7", static_cast<int>(Scancode::F7));
  Binding::registerInteger(L, "F8", static_cast<int>(Scancode::F8));
  Binding::registerInteger(L, "F9", static_cast<int>(Scancode::F9));
  Binding::registerInteger(L, "F10", static_cast<int>(Scancode::F10));
  Binding::registerInteger(L, "F11", static_cast<int>(Scancode::F11));
  Binding::registerInteger(L, "F12", static_cast<int>(Scancode::F12));

  Binding::registerInteger(L, "UP", static_cast<int>(Scancode::UP));
  Binding::registerInteger(L, "DOWN", static_cast<int>(Scancode::DOWN));
  Binding::registerInteger(L, "LEFT", static_cast<int>(Scancode::LEFT));
  Binding::registerInteger(L, "RIGHT", static_cast<int>(Scancode::RIGHT));
  Binding::registerInteger(L, "ESCAPE", static_cast<int>(Scancode::ESCAPE));
  Binding::registerInteger(L, "SPACE", static_cast<int>(Scancode::SPACE));
  Binding::registerInteger(L, "ENTER", static_cast<int>(Scancode::RETURN));
  Binding::registerInteger(L, "BACKSPACE", static_cast<int>(Scancode::BACKSPACE));
  Binding::registerInteger(L, "TAB", static_cast<int>(Scancode::TAB));
  Binding::registerInteger(L, "LSHIFT", static_cast<int>(Scancode::LSHIFT));
  Binding::registerInteger(L, "RSHIFT", static_cast<int>(Scancode::RSHIFT));
  Binding::registerInteger(L, "LCTRL", static_cast<int>(Scancode::LCTRL));
  Binding::registerInteger(L, "RCTRL", static_cast<int>(Scancode::RCTRL));
  Binding::registerInteger(L, "ALT", static_cast<int>(Scancode::ALTERASE));

  lua_pop(L, 1);
}
} // namespace sinen
