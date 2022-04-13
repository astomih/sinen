#include "../event/current_event.hpp"
#include "Input/MouseCode.hpp"
#include "Utility/Singleton.hpp"
#include <Input/InputSystem.hpp>
#include <SDL.h>
#include <Window/Window.hpp>
#include <cstring>
#include <imgui_impl_sdl.h>
#include <manager/manager.hpp>

namespace nen {
bool isHide = false;
SDL_Cursor *g_cursor = nullptr;

bool keyboard_state::GetKeyValue(key_code _key_code) const {
  return mCurrState[static_cast<int>(_key_code)] == 1;
}

button_state keyboard_state::GetKeyState(key_code _key_code) const {
  if (mPrevState[static_cast<int>(_key_code)] == 0) {
    if (mCurrState[static_cast<int>(_key_code)] == 0) {
      return button_state::None;
    } else {
      return button_state::Pressed;
    }
  } else // Prev state must be 1
  {
    if (mCurrState[static_cast<int>(_key_code)] == 0) {
      return button_state::Released;
    } else {
      return button_state::Held;
    }
  }
}

mouse_state::mouse_state(manager &_manager) : m_manager(_manager) {}

void mouse_state::SetPosition(const vector2 &pos) const {
  SDL_WarpMouseInWindow((SDL_Window *)m_manager.get_window().GetSDLWindow(),
                        pos.x, pos.y);
}

void mouse_state::HideCursor(bool hide) const {
  isHide = hide;
  if (hide) {
    SDL_ShowCursor(SDL_DISABLE);
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
  } else {
    SDL_ShowCursor(SDL_ENABLE);
  }
}
bool mouse_state::is_button_down(mouse_code _button) const {
  return (SDL_BUTTON(static_cast<int>(_button)) & mCurrButtons) == 1;
}

button_state mouse_state::get_button_state(mouse_code _button) const {
  int mask = SDL_BUTTON(static_cast<int>(_button));
  if ((mask & mPrevButtons) == 0) {
    if ((mask & mCurrButtons) == 0) {
      return button_state::None;
    } else {
      return button_state::Pressed;
    }
  } else {
    if ((mask & mCurrButtons) == 0) {
      return button_state::Released;
    } else {
      return button_state::Held;
    }
  }
}

bool joystick_state::GetButtonValue(joystick_button _button) const {
  return mCurrButtons[static_cast<int>(_button)] == 1;
}

button_state joystick_state::GetButtonState(joystick_button _button) const {
  if (mPrevButtons[static_cast<int>(_button)] == 0) {
    if (mCurrButtons[static_cast<int>(_button)] == 0) {
      return button_state::None;
    } else {
      return button_state::Pressed;
    }
  } else // Prev state must be 1
  {
    if (mCurrButtons[static_cast<int>(_button)] == 0) {
      return button_state::Released;
    } else {
      return button_state::Held;
    }
  }
}

bool input_system::Initialize() {

  mState.Keyboard.mCurrState = SDL_GetKeyboardState(NULL);
  memcpy(mState.Keyboard.mPrevState.data(), mState.Keyboard.mCurrState,
         SDL_NUM_SCANCODES);

  int x = 0, y = 0;
  if (mState.Mouse.mIsRelative) {
    mState.Mouse.mCurrButtons = SDL_GetRelativeMouseState(&x, &y);
  } else {
    mState.Mouse.mCurrButtons = SDL_GetMouseState(&x, &y);
  }

  mState.Mouse.mMousePos.x = static_cast<float>(x);
  mState.Mouse.mMousePos.y = static_cast<float>(y);

  // Initialize controller state
  mState.Controller.mIsConnected = mController.Initialize();
  memset(mState.Controller.mCurrButtons, 0, SDL_CONTROLLER_BUTTON_MAX);
  memset(mState.Controller.mPrevButtons, 0, SDL_CONTROLLER_BUTTON_MAX);

  return true;
}

void input_system::Shutdown() {
  SDL_FreeCursor(g_cursor);
  g_cursor = nullptr;
  SDL_SetCursor(NULL);
}

void input_system::PrepareForUpdate() {
  // Copy current state to previous
  // Keyboard
  memcpy(mState.Keyboard.mPrevState.data(), mState.Keyboard.mCurrState,
         SDL_NUM_SCANCODES);

  // Mouse
  mState.Mouse.mPrevButtons = mState.Mouse.mCurrButtons;
  mState.Mouse.mIsRelative = false;
  mState.Mouse.mScrollWheel = vector2::Zero;

  // Controller
  memcpy(mState.Controller.mPrevButtons, mState.Controller.mCurrButtons,
         SDL_CONTROLLER_BUTTON_MAX);
}

void input_system::Update() {
  // Mouse
  int x = 0, y = 0;
  if (mState.Mouse.mIsRelative) {
    mState.Mouse.mCurrButtons = SDL_GetRelativeMouseState(&x, &y);
  } else {
    mState.Mouse.mCurrButtons = SDL_GetMouseState(&x, &y);
  }

  mState.Mouse.mMousePos.x = static_cast<float>(x);
  mState.Mouse.mMousePos.y = static_cast<float>(y);

  // Controller
  // Buttons
  for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
    mState.Controller.mCurrButtons[i] =
        mController.GetButton(static_cast<joystick_button>(i));
  }

  // Triggers
  mState.Controller.mLeftTrigger =
      Filter1D(mController.GetAxis(joystick::axis::TRIGGERLEFT));
  mState.Controller.mRightTrigger =
      Filter1D(mController.GetAxis(joystick::axis::TRIGGERRIGHT));

  // Sticks
  x = mController.GetAxis(joystick::axis::LEFTX);
  y = -mController.GetAxis(joystick::axis::LEFTY);
  mState.Controller.mLeftStick = Filter2D(x, y);

  x = mController.GetAxis(joystick::axis::RIGHTX);
  y = -mController.GetAxis(joystick::axis::RIGHTY);
  mState.Controller.mRightStick = Filter2D(x, y);

  mState.Mouse.HideCursor(isHide);
}
SDL_Event current_event_handle::current_event = SDL_Event{};
void input_system::ProcessEvent() {

  auto e = current_event_handle::current_event;
  switch (e.type) {
  case SDL_MOUSEWHEEL: {
    mState.Mouse.mScrollWheel =
        vector2(static_cast<float>(e.wheel.x), static_cast<float>(e.wheel.y));
    break;
  }
  default:
    break;
  }
}

void input_system::SetRelativeMouseMode(bool _value) {
  SDL_bool set = _value ? SDL_TRUE : SDL_FALSE;
  SDL_SetRelativeMouseMode(set);

  mState.Mouse.mIsRelative = _value;
}

float input_system::Filter1D(int _input) {
  // A value < dead zone is interpreted as 0%
  const int deadZone = 250;
  // A value > max value is interpreted as 100%
  const int maxValue = 30000;

  float retVal = 0.0f;

  // Take absolute value of input
  int absValue = _input > 0 ? _input : -_input;
  // Ignore input within dead zone
  if (absValue > deadZone) {
    // Compute fractional value between dead zone and max value
    retVal = static_cast<float>(absValue - deadZone) / (maxValue - deadZone);
    // Make sure sign matches original value
    retVal = _input > 0 ? retVal : -1.0f * retVal;
    // Clamp between -1.0f and 1.0f
    retVal = Math::Clamp(retVal, -1.0f, 1.0f);
  }

  return retVal;
}

vector2 input_system::Filter2D(int inputX, int inputY) {
  const float deadZone = 8000.0f;
  const float maxValue = 30000.0f;

  // Make into 2D vector
  vector2 dir;
  dir.x = static_cast<float>(inputX);
  dir.y = static_cast<float>(inputY);

  float length = dir.Length();

  // If length < deadZone, should be no input
  if (length < deadZone) {
    dir = vector2::Zero;
  } else {
    // Calculate fractional value between
    // dead zone and max value circles
    float f = (length - deadZone) / (maxValue - deadZone);
    // Clamp f between 0.0f and 1.0f
    f = Math::Clamp(f, 0.0f, 1.0f);
    // Normalize the vector, and then scale it to the
    // fractional value
    dir *= f / length;
  }

  return dir;
}
} // namespace nen