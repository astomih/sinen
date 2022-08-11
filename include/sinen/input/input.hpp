#ifndef SINEN_INPUT_HPP
#define SINEN_INPUT_HPP
#include "../event/process_event.hpp"
#include "../math/vector2.hpp"
#include "game_controller.hpp"
#include "game_controller_button.hpp"
#include "key_code.hpp"
#include "mouse_code.hpp"
#include <array>
#include <cstdint>

namespace sinen {
// The different button states
enum class button_state { None, Pressed, Released, Held };

// Helper for keyboard input
class keyboard_state {
public:
  // Get just the boolean true/false value of key
  bool is_key_down(key_code _key) const;
  // Get a state based on current and previous frame
  button_state get_key_state(key_code _key) const;
};

// Helper for mouse input
class mouse_state {
public:
  mouse_state();
  // For mouse position
  void set_position(const vector2 &pos) const;
  void hide_cursor(bool hide) const;
  const vector2 &get_position() const;
  const vector2 &get_scroll_wheel() const;
  bool IsRelative() const;
  // For buttons
  bool is_button_down(mouse_code _button) const;
  button_state get_button_state(mouse_code _button) const;
};

// Helper for controller input
class joystick_state {
public:
  // For buttons
  bool get_button_value(joystick_button j_button) const;
  button_state get_button_state(joystick_button j_button) const;

  const vector2 &get_left_stick() const;
  const vector2 &get_right_stick() const;
  float get_left_trigger() const;
  float get_right_trigger() const;

  bool is_connected() const;
};

class input {
public:
  static keyboard_state keyboard;
  static mouse_state mouse;
  static joystick_state controller;
  static void set_relative_mouse_mode(bool value);
};
} // namespace sinen

#endif // !SINEN_INPUT_HPP
