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
  luau::table(L, -1).raw_function("isPressed", lKeyboardIsPressed);
  luau::table(L, -1).raw_function("isReleased", lKeyboardIsReleased);
  luau::table(L, -1).raw_function("isDown", lKeyboardIsDown);
  lua_pop(L, 1);

  pushSnNamed(L, "Scancode");

  luau::table(L, -1).value("A", static_cast<int>(Scancode::A));
  luau::table(L, -1).value("B", static_cast<int>(Scancode::B));
  luau::table(L, -1).value("C", static_cast<int>(Scancode::C));
  luau::table(L, -1).value("D", static_cast<int>(Scancode::D));
  luau::table(L, -1).value("E", static_cast<int>(Scancode::E));
  luau::table(L, -1).value("F", static_cast<int>(Scancode::F));
  luau::table(L, -1).value("G", static_cast<int>(Scancode::G));
  luau::table(L, -1).value("H", static_cast<int>(Scancode::H));
  luau::table(L, -1).value("I", static_cast<int>(Scancode::I));
  luau::table(L, -1).value("J", static_cast<int>(Scancode::J));
  luau::table(L, -1).value("K", static_cast<int>(Scancode::K));
  luau::table(L, -1).value("L", static_cast<int>(Scancode::L));
  luau::table(L, -1).value("M", static_cast<int>(Scancode::M));
  luau::table(L, -1).value("N", static_cast<int>(Scancode::N));
  luau::table(L, -1).value("O", static_cast<int>(Scancode::O));
  luau::table(L, -1).value("P", static_cast<int>(Scancode::P));
  luau::table(L, -1).value("Q", static_cast<int>(Scancode::Q));
  luau::table(L, -1).value("R", static_cast<int>(Scancode::R));
  luau::table(L, -1).value("S", static_cast<int>(Scancode::S));
  luau::table(L, -1).value("T", static_cast<int>(Scancode::T));
  luau::table(L, -1).value("U", static_cast<int>(Scancode::U));
  luau::table(L, -1).value("V", static_cast<int>(Scancode::V));
  luau::table(L, -1).value("W", static_cast<int>(Scancode::W));
  luau::table(L, -1).value("X", static_cast<int>(Scancode::X));
  luau::table(L, -1).value("Y", static_cast<int>(Scancode::Y));
  luau::table(L, -1).value("Z", static_cast<int>(Scancode::Z));

  luau::table(L, -1).value("key0", static_cast<int>(Scancode::Key0));
  luau::table(L, -1).value("key1", static_cast<int>(Scancode::Key1));
  luau::table(L, -1).value("key2", static_cast<int>(Scancode::Key2));
  luau::table(L, -1).value("key3", static_cast<int>(Scancode::Key3));
  luau::table(L, -1).value("key4", static_cast<int>(Scancode::Key4));
  luau::table(L, -1).value("key5", static_cast<int>(Scancode::Key5));
  luau::table(L, -1).value("key6", static_cast<int>(Scancode::Key6));
  luau::table(L, -1).value("key7", static_cast<int>(Scancode::Key7));
  luau::table(L, -1).value("key8", static_cast<int>(Scancode::Key8));
  luau::table(L, -1).value("key9", static_cast<int>(Scancode::Key9));

  luau::table(L, -1).value("F1", static_cast<int>(Scancode::F1));
  luau::table(L, -1).value("F2", static_cast<int>(Scancode::F2));
  luau::table(L, -1).value("F3", static_cast<int>(Scancode::F3));
  luau::table(L, -1).value("F4", static_cast<int>(Scancode::F4));
  luau::table(L, -1).value("F5", static_cast<int>(Scancode::F5));
  luau::table(L, -1).value("F6", static_cast<int>(Scancode::F6));
  luau::table(L, -1).value("F7", static_cast<int>(Scancode::F7));
  luau::table(L, -1).value("F8", static_cast<int>(Scancode::F8));
  luau::table(L, -1).value("F9", static_cast<int>(Scancode::F9));
  luau::table(L, -1).value("F10", static_cast<int>(Scancode::F10));
  luau::table(L, -1).value("F11", static_cast<int>(Scancode::F11));
  luau::table(L, -1).value("F12", static_cast<int>(Scancode::F12));

  luau::table(L, -1).value("UP", static_cast<int>(Scancode::UP));
  luau::table(L, -1).value("DOWN", static_cast<int>(Scancode::DOWN));
  luau::table(L, -1).value("LEFT", static_cast<int>(Scancode::LEFT));
  luau::table(L, -1).value("RIGHT", static_cast<int>(Scancode::RIGHT));
  luau::table(L, -1).value("ESCAPE", static_cast<int>(Scancode::ESCAPE));
  luau::table(L, -1).value("SPACE", static_cast<int>(Scancode::SPACE));
  luau::table(L, -1).value("ENTER", static_cast<int>(Scancode::RETURN));
  luau::table(L, -1).value("BACKSPACE", static_cast<int>(Scancode::BACKSPACE));
  luau::table(L, -1).value("TAB", static_cast<int>(Scancode::TAB));
  luau::table(L, -1).value("LSHIFT", static_cast<int>(Scancode::LSHIFT));
  luau::table(L, -1).value("RSHIFT", static_cast<int>(Scancode::RSHIFT));
  luau::table(L, -1).value("LCTRL", static_cast<int>(Scancode::LCTRL));
  luau::table(L, -1).value("RCTRL", static_cast<int>(Scancode::RCTRL));
  luau::table(L, -1).value("ALT", static_cast<int>(Scancode::ALTERASE));

  lua_pop(L, 1);
}
} // namespace sinen
