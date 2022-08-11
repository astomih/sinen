#pragma once
#include <input/input.hpp>

namespace sinen {
class keyboard_state_impl {
public:
  const uint8_t *mCurrState;
  std::array<uint8_t, static_cast<int>(key_code::NUM_KEYCODES)> mPrevState;
};

class mouse_state_impl {
public:
  // Store current mouse position
  vector2 mMousePos;
  vector2 next_pos;
  // Motion of scroll wheel
  vector2 mScrollWheel;
  // Store button data
  uint32_t mCurrButtons;
  uint32_t mPrevButtons;
  // Are we in relative mouse mode
  bool mIsRelative;
  bool is_update_pos = false;
};

class joystick_state_impl {
public:
  // Current/previous buttons
  uint8_t mCurrButtons[static_cast<int>(
      joystick_button::NUM_GAMECONTROLLER_BUTTON)];
  uint8_t mPrevButtons[static_cast<int>(
      joystick_button::NUM_GAMECONTROLLER_BUTTON)];
  // Left/right sticks
  vector2 mLeftStick;
  vector2 mRightStick;
  // Left/right trigger
  float mLeftTrigger;
  float mRightTrigger;
  // Is this controller connected?
  bool mIsConnected;
};

class input_system {
public:
  friend class keyboard_state;
  friend class mouse_state;
  friend class joystick_state;
  bool initialize();
  void terminate();

  // Called right before SDL_PollEvents loop
  void prepare_for_update();
  // Called after SDL_PollEvents loop
  void update();
  void process_event();

  void set_relative_mouse_mode(bool value);

  static keyboard_state_impl m_keyboard;
  static mouse_state_impl m_mouse;
  static joystick_state_impl m_joystick;

private:
  float filter1d(int input);
  vector2 filter2d(int inputX, int inputY);
  joystick mController;
};
} // namespace sinen
