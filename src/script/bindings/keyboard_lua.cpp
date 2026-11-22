#include "luaapi.hpp"
#include <platform/input/keyboard.hpp>

namespace sinen {
void registerKeyboard(luau::table &sn) {
  auto keyboard = sn.get_or_create_table("Keyboard");
  keyboard.function<&Keyboard::isPressed>("isPressed");
  keyboard.function<&Keyboard::isReleased>("isReleased");
  keyboard.function<&Keyboard::isDown>("isDown");

  auto scancode = sn.get_or_create_table("Scancode");

  scancode.value("A", static_cast<int>(Scancode::A));
  scancode.value("B", static_cast<int>(Scancode::B));
  scancode.value("C", static_cast<int>(Scancode::C));
  scancode.value("D", static_cast<int>(Scancode::D));
  scancode.value("E", static_cast<int>(Scancode::E));
  scancode.value("F", static_cast<int>(Scancode::F));
  scancode.value("G", static_cast<int>(Scancode::G));
  scancode.value("H", static_cast<int>(Scancode::H));
  scancode.value("I", static_cast<int>(Scancode::I));
  scancode.value("J", static_cast<int>(Scancode::J));
  scancode.value("K", static_cast<int>(Scancode::K));
  scancode.value("L", static_cast<int>(Scancode::L));
  scancode.value("M", static_cast<int>(Scancode::M));
  scancode.value("N", static_cast<int>(Scancode::N));
  scancode.value("O", static_cast<int>(Scancode::O));
  scancode.value("P", static_cast<int>(Scancode::P));
  scancode.value("Q", static_cast<int>(Scancode::Q));
  scancode.value("R", static_cast<int>(Scancode::R));
  scancode.value("S", static_cast<int>(Scancode::S));
  scancode.value("T", static_cast<int>(Scancode::T));
  scancode.value("U", static_cast<int>(Scancode::U));
  scancode.value("V", static_cast<int>(Scancode::V));
  scancode.value("W", static_cast<int>(Scancode::W));
  scancode.value("X", static_cast<int>(Scancode::X));
  scancode.value("Y", static_cast<int>(Scancode::Y));
  scancode.value("Z", static_cast<int>(Scancode::Z));

  scancode.value("key0", static_cast<int>(Scancode::Key0));
  scancode.value("key1", static_cast<int>(Scancode::Key1));
  scancode.value("key2", static_cast<int>(Scancode::Key2));
  scancode.value("key3", static_cast<int>(Scancode::Key3));
  scancode.value("key4", static_cast<int>(Scancode::Key4));
  scancode.value("key5", static_cast<int>(Scancode::Key5));
  scancode.value("key6", static_cast<int>(Scancode::Key6));
  scancode.value("key7", static_cast<int>(Scancode::Key7));
  scancode.value("key8", static_cast<int>(Scancode::Key8));
  scancode.value("key9", static_cast<int>(Scancode::Key9));

  scancode.value("F1", static_cast<int>(Scancode::F1));
  scancode.value("F2", static_cast<int>(Scancode::F2));
  scancode.value("F3", static_cast<int>(Scancode::F3));
  scancode.value("F4", static_cast<int>(Scancode::F4));
  scancode.value("F5", static_cast<int>(Scancode::F5));
  scancode.value("F6", static_cast<int>(Scancode::F6));
  scancode.value("F7", static_cast<int>(Scancode::F7));
  scancode.value("F8", static_cast<int>(Scancode::F8));
  scancode.value("F9", static_cast<int>(Scancode::F9));
  scancode.value("F10", static_cast<int>(Scancode::F10));
  scancode.value("F11", static_cast<int>(Scancode::F11));
  scancode.value("F12", static_cast<int>(Scancode::F12));

  scancode.value("UP", static_cast<int>(Scancode::UP));
  scancode.value("DOWN", static_cast<int>(Scancode::DOWN));
  scancode.value("LEFT", static_cast<int>(Scancode::LEFT));
  scancode.value("RIGHT", static_cast<int>(Scancode::RIGHT));
  scancode.value("ESCAPE", static_cast<int>(Scancode::ESCAPE));
  scancode.value("SPACE", static_cast<int>(Scancode::SPACE));
  scancode.value("ENTER", static_cast<int>(Scancode::RETURN));
  scancode.value("BACKSPACE", static_cast<int>(Scancode::BACKSPACE));
  scancode.value("TAB", static_cast<int>(Scancode::TAB));
  scancode.value("LSHIFT", static_cast<int>(Scancode::LSHIFT));
  scancode.value("RSHIFT", static_cast<int>(Scancode::RSHIFT));
  scancode.value("LCTRL", static_cast<int>(Scancode::LCTRL));
  scancode.value("RCTRL", static_cast<int>(Scancode::RCTRL));
  scancode.value("ALT", static_cast<int>(Scancode::ALTERASE));
}
} // namespace sinen
