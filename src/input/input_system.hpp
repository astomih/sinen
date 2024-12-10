#ifndef SINEN_INPUT_SYSTEM_HPP
#define SINEN_INPUT_SYSTEM_HPP
#include "game_controller.hpp"
#include <array>
#include <cstdint>
#include <input/gamepad.hpp>
#include <input/keyboard.hpp>
#include <input/mouse.hpp>

#include <SDL.h>
namespace sinen {
struct keyboard_state_impl {
public:
  const uint8_t *mCurrState;
  std::array<uint8_t, static_cast<int>(Keyboard::code::NUM_KEYCODES)>
      mPrevState;
};

struct mouse_state_impl {
public:
  // Store current mouse position
  Vector2 mMousePos;
  // Motion of scroll wheel
  Vector2 mScrollWheel;
  // Store button data
  uint32_t mCurrButtons;
  uint32_t mPrevButtons;
  // Are we in relative mouse mode
  bool mIsRelative;
};

struct joystick_state_impl {
public:
  // Current/previous buttons
  uint8_t
      mCurrButtons[static_cast<int>(GamePad::code::NUM_GAMECONTROLLER_BUTTON)];
  uint8_t
      mPrevButtons[static_cast<int>(GamePad::code::NUM_GAMECONTROLLER_BUTTON)];
  // Left/right sticks
  Vector2 mLeftStick;
  Vector2 mRightStick;
  // Left/right trigger
  float mLeftTrigger;
  float mRightTrigger;
  // Is this controller connected?
  bool mIsConnected;
};

class input_system {
public:
  static bool initialize();
  static void shutdown();

  // Called right before SDL_PollEvents loop
  static void prepare_for_update();
  // Called after SDL_PollEvents loop
  static void update();
  static void process_event(SDL_Event &event);

  static void set_relative_mouse_mode(bool value);

  static keyboard_state_impl m_keyboard;
  static mouse_state_impl m_mouse;
  static joystick_state_impl m_joystick;

private:
  static float filter1d(int input);
  static Vector2 filter2d(int inputX, int inputY);
  static joystick mController;
};
} // namespace sinen
#endif // !SINEN_INPUT_SYSTEM_HPP