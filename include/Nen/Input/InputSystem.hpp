#pragma once
#include "../Math/Vector2.hpp"
#include "../Render/Renderer.hpp"
#include "../event/process_event.hpp"
#include "GameController.hpp"
#include "GameControllerButton.hpp"
#include "KeyCode.hpp"
#include "MouseCode.hpp"
#include <array>
#include <cstdint>

namespace nen {
// The different button states
enum class button_state { None, Pressed, Released, Held };

// Helper for keyboard input
class keyboard_state {
public:
  // Friend so InputSystem can easily update it
  friend class input_system;
  // Get just the boolean true/false value of key
  bool GetKeyValue(key_code _key) const;
  // Get a state based on current and previous frame
  button_state GetKeyState(key_code _key) const;

private:
  const uint8_t *mCurrState;
  std::array<uint8_t, static_cast<int>(key_code::NUM_KEYCODES)> mPrevState;
};

// Helper for mouse input
class mouse_state {
public:
  mouse_state(renderer &_renderer);
  friend class input_system;

  // For mouse position
  void SetPosition(const vector2 &pos) const;
  void HideCursor(bool hide) const;
  const vector2 &GetPosition() const { return mMousePos; }
  const vector2 &GetScrollWheel() const { return mScrollWheel; }
  bool IsRelative() const { return mIsRelative; }

  // For buttons
  bool GetButtonValue(mouse_code _button) const;
  button_state GetButtonState(mouse_code _button) const;

private:
  renderer &m_renderer;
  // Store current mouse position
  vector2 mMousePos;
  // Motion of scroll wheel
  vector2 mScrollWheel;
  // Store button data
  uint32_t mCurrButtons;
  uint32_t mPrevButtons;
  // Are we in relative mouse mode
  bool mIsRelative;
};

// Helper for controller input
class joystick_state {
public:
  friend class input_system;

  // For buttons
  bool GetButtonValue(joystick_button j_button) const;
  button_state GetButtonState(joystick_button j_button) const;

  const vector2 &GetLeftStick() const { return mLeftStick; }
  const vector2 &GetRightStick() const { return mRightStick; }
  float GetLeftTrigger() const { return mLeftTrigger; }
  float GetRightTrigger() const { return mRightTrigger; }

  bool GetIsConnected() const { return mIsConnected; }

private:
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

// Wrapper that contains current state of input
struct input_state {
  input_state(renderer &_renderer) : Mouse(_renderer) {}
  keyboard_state Keyboard;
  mouse_state Mouse;
  joystick_state Controller;
};

class input_system {
public:
  input_system(renderer &_renderer)
      : m_renderer(_renderer), mState(_renderer) {}
  bool Initialize();
  void Shutdown();

  // Called right before SDL_PollEvents loop
  void PrepareForUpdate();
  // Called after SDL_PollEvents loop
  void Update();
  void ProcessEvent();

  const input_state &GetState() const { return mState; }

  void SetRelativeMouseMode(bool value);

private:
  renderer &m_renderer;
  float Filter1D(int input);
  vector2 Filter2D(int inputX, int inputY);
  input_state mState;
  joystick mController;
};
} // namespace nen
