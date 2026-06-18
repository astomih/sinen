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
  luaPushcfunction2(L, lKeyboardIsPressed);
  lua_setfield(L, -2, "isPressed");
  luaPushcfunction2(L, lKeyboardIsReleased);
  lua_setfield(L, -2, "isReleased");
  luaPushcfunction2(L, lKeyboardIsDown);
  lua_setfield(L, -2, "isDown");
  lua_pop(L, 1);

  pushSnNamed(L, "Scancode");

  lua_pushinteger(L, static_cast<int>(Scancode::A));
  lua_setfield(L, -2, "A");
  lua_pushinteger(L, static_cast<int>(Scancode::B));
  lua_setfield(L, -2, "B");
  lua_pushinteger(L, static_cast<int>(Scancode::C));
  lua_setfield(L, -2, "C");
  lua_pushinteger(L, static_cast<int>(Scancode::D));
  lua_setfield(L, -2, "D");
  lua_pushinteger(L, static_cast<int>(Scancode::E));
  lua_setfield(L, -2, "E");
  lua_pushinteger(L, static_cast<int>(Scancode::F));
  lua_setfield(L, -2, "F");
  lua_pushinteger(L, static_cast<int>(Scancode::G));
  lua_setfield(L, -2, "G");
  lua_pushinteger(L, static_cast<int>(Scancode::H));
  lua_setfield(L, -2, "H");
  lua_pushinteger(L, static_cast<int>(Scancode::I));
  lua_setfield(L, -2, "I");
  lua_pushinteger(L, static_cast<int>(Scancode::J));
  lua_setfield(L, -2, "J");
  lua_pushinteger(L, static_cast<int>(Scancode::K));
  lua_setfield(L, -2, "K");
  lua_pushinteger(L, static_cast<int>(Scancode::L));
  lua_setfield(L, -2, "L");
  lua_pushinteger(L, static_cast<int>(Scancode::M));
  lua_setfield(L, -2, "M");
  lua_pushinteger(L, static_cast<int>(Scancode::N));
  lua_setfield(L, -2, "N");
  lua_pushinteger(L, static_cast<int>(Scancode::O));
  lua_setfield(L, -2, "O");
  lua_pushinteger(L, static_cast<int>(Scancode::P));
  lua_setfield(L, -2, "P");
  lua_pushinteger(L, static_cast<int>(Scancode::Q));
  lua_setfield(L, -2, "Q");
  lua_pushinteger(L, static_cast<int>(Scancode::R));
  lua_setfield(L, -2, "R");
  lua_pushinteger(L, static_cast<int>(Scancode::S));
  lua_setfield(L, -2, "S");
  lua_pushinteger(L, static_cast<int>(Scancode::T));
  lua_setfield(L, -2, "T");
  lua_pushinteger(L, static_cast<int>(Scancode::U));
  lua_setfield(L, -2, "U");
  lua_pushinteger(L, static_cast<int>(Scancode::V));
  lua_setfield(L, -2, "V");
  lua_pushinteger(L, static_cast<int>(Scancode::W));
  lua_setfield(L, -2, "W");
  lua_pushinteger(L, static_cast<int>(Scancode::X));
  lua_setfield(L, -2, "X");
  lua_pushinteger(L, static_cast<int>(Scancode::Y));
  lua_setfield(L, -2, "Y");
  lua_pushinteger(L, static_cast<int>(Scancode::Z));
  lua_setfield(L, -2, "Z");

  lua_pushinteger(L, static_cast<int>(Scancode::Key0));
  lua_setfield(L, -2, "key0");
  lua_pushinteger(L, static_cast<int>(Scancode::Key1));
  lua_setfield(L, -2, "key1");
  lua_pushinteger(L, static_cast<int>(Scancode::Key2));
  lua_setfield(L, -2, "key2");
  lua_pushinteger(L, static_cast<int>(Scancode::Key3));
  lua_setfield(L, -2, "key3");
  lua_pushinteger(L, static_cast<int>(Scancode::Key4));
  lua_setfield(L, -2, "key4");
  lua_pushinteger(L, static_cast<int>(Scancode::Key5));
  lua_setfield(L, -2, "key5");
  lua_pushinteger(L, static_cast<int>(Scancode::Key6));
  lua_setfield(L, -2, "key6");
  lua_pushinteger(L, static_cast<int>(Scancode::Key7));
  lua_setfield(L, -2, "key7");
  lua_pushinteger(L, static_cast<int>(Scancode::Key8));
  lua_setfield(L, -2, "key8");
  lua_pushinteger(L, static_cast<int>(Scancode::Key9));
  lua_setfield(L, -2, "key9");

  lua_pushinteger(L, static_cast<int>(Scancode::F1));
  lua_setfield(L, -2, "F1");
  lua_pushinteger(L, static_cast<int>(Scancode::F2));
  lua_setfield(L, -2, "F2");
  lua_pushinteger(L, static_cast<int>(Scancode::F3));
  lua_setfield(L, -2, "F3");
  lua_pushinteger(L, static_cast<int>(Scancode::F4));
  lua_setfield(L, -2, "F4");
  lua_pushinteger(L, static_cast<int>(Scancode::F5));
  lua_setfield(L, -2, "F5");
  lua_pushinteger(L, static_cast<int>(Scancode::F6));
  lua_setfield(L, -2, "F6");
  lua_pushinteger(L, static_cast<int>(Scancode::F7));
  lua_setfield(L, -2, "F7");
  lua_pushinteger(L, static_cast<int>(Scancode::F8));
  lua_setfield(L, -2, "F8");
  lua_pushinteger(L, static_cast<int>(Scancode::F9));
  lua_setfield(L, -2, "F9");
  lua_pushinteger(L, static_cast<int>(Scancode::F10));
  lua_setfield(L, -2, "F10");
  lua_pushinteger(L, static_cast<int>(Scancode::F11));
  lua_setfield(L, -2, "F11");
  lua_pushinteger(L, static_cast<int>(Scancode::F12));
  lua_setfield(L, -2, "F12");

  lua_pushinteger(L, static_cast<int>(Scancode::UP));
  lua_setfield(L, -2, "UP");
  lua_pushinteger(L, static_cast<int>(Scancode::DOWN));
  lua_setfield(L, -2, "DOWN");
  lua_pushinteger(L, static_cast<int>(Scancode::LEFT));
  lua_setfield(L, -2, "LEFT");
  lua_pushinteger(L, static_cast<int>(Scancode::RIGHT));
  lua_setfield(L, -2, "RIGHT");
  lua_pushinteger(L, static_cast<int>(Scancode::ESCAPE));
  lua_setfield(L, -2, "ESCAPE");
  lua_pushinteger(L, static_cast<int>(Scancode::SPACE));
  lua_setfield(L, -2, "SPACE");
  lua_pushinteger(L, static_cast<int>(Scancode::RETURN));
  lua_setfield(L, -2, "ENTER");
  lua_pushinteger(L, static_cast<int>(Scancode::BACKSPACE));
  lua_setfield(L, -2, "BACKSPACE");
  lua_pushinteger(L, static_cast<int>(Scancode::TAB));
  lua_setfield(L, -2, "TAB");
  lua_pushinteger(L, static_cast<int>(Scancode::LSHIFT));
  lua_setfield(L, -2, "LSHIFT");
  lua_pushinteger(L, static_cast<int>(Scancode::RSHIFT));
  lua_setfield(L, -2, "RSHIFT");
  lua_pushinteger(L, static_cast<int>(Scancode::LCTRL));
  lua_setfield(L, -2, "LCTRL");
  lua_pushinteger(L, static_cast<int>(Scancode::RCTRL));
  lua_setfield(L, -2, "RCTRL");
  lua_pushinteger(L, static_cast<int>(Scancode::ALTERASE));
  lua_setfield(L, -2, "ALT");

  lua_pop(L, 1);
}
} // namespace sinen
