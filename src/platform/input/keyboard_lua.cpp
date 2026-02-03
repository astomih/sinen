#include <platform/input/keyboard.hpp>
#include <script/luaapi.hpp>

namespace sinen {
static int lKeyboardIsPressed(lua_State *L) {
  auto key = static_cast<Keyboard::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Keyboard::isPressed(key));
  return 1;
}
static int lKeyboardIsReleased(lua_State *L) {
  auto key = static_cast<Keyboard::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Keyboard::isReleased(key));
  return 1;
}
static int lKeyboardIsDown(lua_State *L) {
  auto key = static_cast<Keyboard::Code>(luaL_checkinteger(L, 1));
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

  lua_pushinteger(L, static_cast<int>(Keyboard::A));
  lua_setfield(L, -2, "A");
  lua_pushinteger(L, static_cast<int>(Keyboard::B));
  lua_setfield(L, -2, "B");
  lua_pushinteger(L, static_cast<int>(Keyboard::C));
  lua_setfield(L, -2, "C");
  lua_pushinteger(L, static_cast<int>(Keyboard::D));
  lua_setfield(L, -2, "D");
  lua_pushinteger(L, static_cast<int>(Keyboard::E));
  lua_setfield(L, -2, "E");
  lua_pushinteger(L, static_cast<int>(Keyboard::F));
  lua_setfield(L, -2, "F");
  lua_pushinteger(L, static_cast<int>(Keyboard::G));
  lua_setfield(L, -2, "G");
  lua_pushinteger(L, static_cast<int>(Keyboard::H));
  lua_setfield(L, -2, "H");
  lua_pushinteger(L, static_cast<int>(Keyboard::I));
  lua_setfield(L, -2, "I");
  lua_pushinteger(L, static_cast<int>(Keyboard::J));
  lua_setfield(L, -2, "J");
  lua_pushinteger(L, static_cast<int>(Keyboard::K));
  lua_setfield(L, -2, "K");
  lua_pushinteger(L, static_cast<int>(Keyboard::L));
  lua_setfield(L, -2, "L");
  lua_pushinteger(L, static_cast<int>(Keyboard::M));
  lua_setfield(L, -2, "M");
  lua_pushinteger(L, static_cast<int>(Keyboard::N));
  lua_setfield(L, -2, "N");
  lua_pushinteger(L, static_cast<int>(Keyboard::O));
  lua_setfield(L, -2, "O");
  lua_pushinteger(L, static_cast<int>(Keyboard::P));
  lua_setfield(L, -2, "P");
  lua_pushinteger(L, static_cast<int>(Keyboard::Q));
  lua_setfield(L, -2, "Q");
  lua_pushinteger(L, static_cast<int>(Keyboard::R));
  lua_setfield(L, -2, "R");
  lua_pushinteger(L, static_cast<int>(Keyboard::S));
  lua_setfield(L, -2, "S");
  lua_pushinteger(L, static_cast<int>(Keyboard::T));
  lua_setfield(L, -2, "T");
  lua_pushinteger(L, static_cast<int>(Keyboard::U));
  lua_setfield(L, -2, "U");
  lua_pushinteger(L, static_cast<int>(Keyboard::V));
  lua_setfield(L, -2, "V");
  lua_pushinteger(L, static_cast<int>(Keyboard::W));
  lua_setfield(L, -2, "W");
  lua_pushinteger(L, static_cast<int>(Keyboard::X));
  lua_setfield(L, -2, "X");
  lua_pushinteger(L, static_cast<int>(Keyboard::Y));
  lua_setfield(L, -2, "Y");
  lua_pushinteger(L, static_cast<int>(Keyboard::Z));
  lua_setfield(L, -2, "Z");

  lua_pushinteger(L, static_cast<int>(Keyboard::Key0));
  lua_setfield(L, -2, "key0");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key1));
  lua_setfield(L, -2, "key1");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key2));
  lua_setfield(L, -2, "key2");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key3));
  lua_setfield(L, -2, "key3");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key4));
  lua_setfield(L, -2, "key4");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key5));
  lua_setfield(L, -2, "key5");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key6));
  lua_setfield(L, -2, "key6");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key7));
  lua_setfield(L, -2, "key7");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key8));
  lua_setfield(L, -2, "key8");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key9));
  lua_setfield(L, -2, "key9");

  lua_pushinteger(L, static_cast<int>(Keyboard::F1));
  lua_setfield(L, -2, "F1");
  lua_pushinteger(L, static_cast<int>(Keyboard::F2));
  lua_setfield(L, -2, "F2");
  lua_pushinteger(L, static_cast<int>(Keyboard::F3));
  lua_setfield(L, -2, "F3");
  lua_pushinteger(L, static_cast<int>(Keyboard::F4));
  lua_setfield(L, -2, "F4");
  lua_pushinteger(L, static_cast<int>(Keyboard::F5));
  lua_setfield(L, -2, "F5");
  lua_pushinteger(L, static_cast<int>(Keyboard::F6));
  lua_setfield(L, -2, "F6");
  lua_pushinteger(L, static_cast<int>(Keyboard::F7));
  lua_setfield(L, -2, "F7");
  lua_pushinteger(L, static_cast<int>(Keyboard::F8));
  lua_setfield(L, -2, "F8");
  lua_pushinteger(L, static_cast<int>(Keyboard::F9));
  lua_setfield(L, -2, "F9");
  lua_pushinteger(L, static_cast<int>(Keyboard::F10));
  lua_setfield(L, -2, "F10");
  lua_pushinteger(L, static_cast<int>(Keyboard::F11));
  lua_setfield(L, -2, "F11");
  lua_pushinteger(L, static_cast<int>(Keyboard::F12));
  lua_setfield(L, -2, "F12");

  lua_pushinteger(L, static_cast<int>(Keyboard::UP));
  lua_setfield(L, -2, "UP");
  lua_pushinteger(L, static_cast<int>(Keyboard::DOWN));
  lua_setfield(L, -2, "DOWN");
  lua_pushinteger(L, static_cast<int>(Keyboard::LEFT));
  lua_setfield(L, -2, "LEFT");
  lua_pushinteger(L, static_cast<int>(Keyboard::RIGHT));
  lua_setfield(L, -2, "RIGHT");
  lua_pushinteger(L, static_cast<int>(Keyboard::ESCAPE));
  lua_setfield(L, -2, "ESCAPE");
  lua_pushinteger(L, static_cast<int>(Keyboard::SPACE));
  lua_setfield(L, -2, "SPACE");
  lua_pushinteger(L, static_cast<int>(Keyboard::RETURN));
  lua_setfield(L, -2, "ENTER");
  lua_pushinteger(L, static_cast<int>(Keyboard::BACKSPACE));
  lua_setfield(L, -2, "BACKSPACE");
  lua_pushinteger(L, static_cast<int>(Keyboard::TAB));
  lua_setfield(L, -2, "TAB");
  lua_pushinteger(L, static_cast<int>(Keyboard::LSHIFT));
  lua_setfield(L, -2, "LSHIFT");
  lua_pushinteger(L, static_cast<int>(Keyboard::RSHIFT));
  lua_setfield(L, -2, "RSHIFT");
  lua_pushinteger(L, static_cast<int>(Keyboard::LCTRL));
  lua_setfield(L, -2, "LCTRL");
  lua_pushinteger(L, static_cast<int>(Keyboard::RCTRL));
  lua_setfield(L, -2, "RCTRL");
  lua_pushinteger(L, static_cast<int>(Keyboard::ALTERASE));
  lua_setfield(L, -2, "ALT");

  lua_pop(L, 1);
}
} // namespace sinen
