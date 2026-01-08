#ifndef SINEN_INPUT_SYSTEM_HPP
#define SINEN_INPUT_SYSTEM_HPP
#include "game_controller.hpp"
#include <array>
#include <cstdint>
#include <platform/platform.hpp>
#include <unordered_set>

#include <SDL3/SDL.h>
namespace sinen {
struct KeyboardStateImpl {
public:
  const bool *mCurrState;
  std::array<uint8_t, static_cast<int>(Keyboard::Code::COUNT)> mPrevState;
};

struct KeyInputState {
  std::unordered_set<std::uint32_t> previousKeys;
  std::unordered_set<std::uint32_t> currentKeys;
};

struct MouseStateImpl {
public:
  // Motion of scroll wheel
  Vec2 mScrollWheel;
  // Store button data
  uint32_t mCurrButtons;
  uint32_t mPrevButtons;
};

struct JoystickStateImpl {
public:
  // Current/previous buttons
  uint8_t
      mCurrButtons[static_cast<int>(GamePad::code::NUM_GAMECONTROLLER_BUTTON)];
  uint8_t
      mPrevButtons[static_cast<int>(GamePad::code::NUM_GAMECONTROLLER_BUTTON)];
  // Left/right sticks
  Vec2 mLeftStick;
  Vec2 mRightStick;
  // Left/right trigger
  float mLeftTrigger;
  float mRightTrigger;
  // Is this controller connected?
  bool mIsConnected;
};

class Input {
public:
  static bool initialize();
  static void shutdown();

  // Called right before SDL_PollEvents loop
  static void prepareForUpdate();
  // Called after SDL_PollEvents loop
  static void update();
  static void processEvent(SDL_Event &event);

  static void setRelativeMouseMode(bool value);

  static KeyboardStateImpl mKeyboard;
  static KeyInputState keyInputState;
  static MouseStateImpl mMouse;
  static JoystickStateImpl mJoystick;

private:
  static float filter1d(int input);
  static Vec2 filter2d(int inputX, int inputY);
  static Joystick mController;
};
} // namespace sinen
#endif // !SINEN_INPUT_SYSTEM_HPP