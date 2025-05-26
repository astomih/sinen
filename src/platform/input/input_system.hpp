#ifndef SINEN_INPUT_SYSTEM_HPP
#define SINEN_INPUT_SYSTEM_HPP
#include "game_controller.hpp"
#include <array>
#include <cstdint>
#include <platform/platform.hpp>
#include <unordered_set>

#include <SDL3/SDL.h>
namespace sinen {
struct keyboard_state_impl {
public:
  const bool *mCurrState;
  std::array<uint8_t, static_cast<int>(Keyboard::code::NUM_KEYCODES)>
      mPrevState;
};

struct KeyInputState {
  std::unordered_set<std::uint32_t> previousKeys;
  std::unordered_set<std::uint32_t> currentKeys;
};

struct mouse_state_impl {
public:
  // Motion of scroll wheel
  glm::vec2 mScrollWheel;
  // Store button data
  uint32_t mCurrButtons;
  uint32_t mPrevButtons;
};

struct joystick_state_impl {
public:
  // Current/previous buttons
  uint8_t
      mCurrButtons[static_cast<int>(GamePad::code::NUM_GAMECONTROLLER_BUTTON)];
  uint8_t
      mPrevButtons[static_cast<int>(GamePad::code::NUM_GAMECONTROLLER_BUTTON)];
  // Left/right sticks
  glm::vec2 mLeftStick;
  glm::vec2 mRightStick;
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
  static KeyInputState keyInputState;
  static mouse_state_impl m_mouse;
  static joystick_state_impl m_joystick;

private:
  static float filter1d(int input);
  static glm::vec2 filter2d(int inputX, int inputY);
  static joystick mController;
};
} // namespace sinen
#endif // !SINEN_INPUT_SYSTEM_HPP