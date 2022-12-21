#include "../event/event_system.hpp"
#include "../window/window_system.hpp"
#include "input_system.hpp"
#include <SDL.h>
#include <cstring>
#include <imgui_impl_sdl.h>
#include <input/input.hpp>
#include <input/mouse_code.hpp>
#include <window/window.hpp>

namespace sinen {
bool isHide = false;
SDL_Cursor *g_cursor = nullptr;

keyboard_state input::keyboard = keyboard_state();
mouse_state input::mouse = mouse_state();
joystick_state input::controller = joystick_state();
keyboard_state_impl input_system::m_keyboard = keyboard_state_impl();
mouse_state_impl input_system::m_mouse = mouse_state_impl();
joystick_state_impl input_system::m_joystick = joystick_state_impl();

bool keyboard_state::is_key_down(key_code _key_code) const {
  return get_key_state(_key_code) == button_state::Held;
}

bool keyboard_state::is_key_pressed(key_code _key_code) const {
  return get_key_state(_key_code) == button_state::Pressed;
}

bool keyboard_state::is_key_released(key_code _key_code) const {
  return get_key_state(_key_code) == button_state::Released;
}

button_state keyboard_state::get_key_state(key_code _key_code) const {
  if (input_system::m_keyboard.mPrevState[static_cast<int>(_key_code)] == 0) {
    if (input_system::m_keyboard.mCurrState[static_cast<int>(_key_code)] == 0) {
      return button_state::None;
    } else {
      return button_state::Pressed;
    }
  } else // Prev state must be 1
  {
    if (input_system::m_keyboard.mCurrState[static_cast<int>(_key_code)] == 0) {
      return button_state::Released;
    } else {
      return button_state::Held;
    }
  }
}

mouse_state::mouse_state() = default;

void mouse_state::set_position(const vector2 &pos) const {
  input_system::m_mouse.is_update_pos = true;
  input_system::m_mouse.next_pos = pos;
  SDL_WarpMouseInWindow(window_system::get_sdl_window(),
                        input_system::m_mouse.next_pos.x,
                        input_system::m_mouse.next_pos.y);
}

const vector2 &mouse_state::get_position() const {

  int x = 0, y = 0;
  if (input_system::m_mouse.mIsRelative) {
    SDL_GetRelativeMouseState(&x, &y);
  } else {
    SDL_GetMouseState(&x, &y);
  }
  input_system::m_mouse.mMousePos.x = static_cast<float>(x);
  input_system::m_mouse.mMousePos.y = static_cast<float>(y);
  return input_system::m_mouse.mMousePos;
}

const vector2 &mouse_state::get_scroll_wheel() const {
  return input_system::m_mouse.mScrollWheel;
}

void mouse_state::hide_cursor(bool hide) const {
  isHide = hide;
  if (hide) {
    SDL_ShowCursor(SDL_DISABLE);
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
  } else {
    SDL_ShowCursor(SDL_ENABLE);
  }
}
bool mouse_state::is_button_down(mouse_code _button) const {
  return get_button_state(_button) == button_state::Held;
}

bool mouse_state::is_button_pressed(mouse_code _button) const {
  return get_button_state(_button) == button_state::Pressed;
}

bool mouse_state::is_button_released(mouse_code _button) const {
  return get_button_state(_button) == button_state::Released;
}

button_state mouse_state::get_button_state(mouse_code _button) const {
  int mask = SDL_BUTTON(static_cast<int>(_button));
  if ((mask & input_system::m_mouse.mPrevButtons) == 0) {
    if ((mask & input_system::m_mouse.mCurrButtons) == 0) {
      return button_state::None;
    } else {
      return button_state::Pressed;
    }
  } else {
    if ((mask & input_system::m_mouse.mCurrButtons) == 0) {
      return button_state::Released;
    } else {
      return button_state::Held;
    }
  }
}
bool joystick_state::is_button_down(joystick_button _button) const {
  return get_button_state(_button) == button_state::Held;
}

bool joystick_state::is_button_pressed(joystick_button _button) const {
  return get_button_state(_button) == button_state::Pressed;
}

bool joystick_state::is_button_released(joystick_button _button) const {
  return get_button_state(_button) == button_state::Released;
}

button_state joystick_state::get_button_state(joystick_button _button) const {
  if (input_system::m_joystick.mPrevButtons[static_cast<int>(_button)] == 0) {
    if (input_system::m_joystick.mCurrButtons[static_cast<int>(_button)] == 0) {
      return button_state::None;
    } else {
      return button_state::Pressed;
    }
  } else // Prev state must be 1
  {
    if (input_system::m_joystick.mCurrButtons[static_cast<int>(_button)] == 0) {
      return button_state::Released;
    } else {
      return button_state::Held;
    }
  }
}

joystick input_system::mController;
bool input_system::initialize() {

  m_keyboard.mCurrState = SDL_GetKeyboardState(NULL);
  memcpy(m_keyboard.mPrevState.data(), m_keyboard.mCurrState,
         SDL_NUM_SCANCODES);

  int x = 0, y = 0;
  if (m_mouse.mIsRelative) {
    m_mouse.mCurrButtons = SDL_GetRelativeMouseState(&x, &y);
  } else {
    m_mouse.mCurrButtons = SDL_GetMouseState(&x, &y);
  }

  m_mouse.mMousePos.x = static_cast<float>(x);
  m_mouse.mMousePos.y = static_cast<float>(y);

  // Initialize controller state
  m_joystick.mIsConnected = mController.initialize();
  memset(m_joystick.mCurrButtons, 0, SDL_CONTROLLER_BUTTON_MAX);
  memset(m_joystick.mPrevButtons, 0, SDL_CONTROLLER_BUTTON_MAX);

  return true;
}

void input_system::shutdown() {}

void input_system::prepare_for_update() {
  // Copy current state to previous
  // Keyboard
  memcpy(m_keyboard.mPrevState.data(), m_keyboard.mCurrState,
         SDL_NUM_SCANCODES);

  // Mouse
  m_mouse.mPrevButtons = m_mouse.mCurrButtons;
  m_mouse.mIsRelative = false;
  m_mouse.mScrollWheel = vector2::zero;

  // Controller
  memcpy(m_joystick.mPrevButtons, m_joystick.mCurrButtons,
         SDL_CONTROLLER_BUTTON_MAX);
}

void input_system::update() {
  // Mouse
  int x = 0, y = 0;
  if (m_mouse.mIsRelative) {
    m_mouse.mCurrButtons = SDL_GetRelativeMouseState(&x, &y);
  } else {
    m_mouse.mCurrButtons = SDL_GetMouseState(&x, &y);
  }

  // Controller
  // Buttons
  for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
    m_joystick.mCurrButtons[i] =
        mController.get_button(static_cast<joystick_button>(i));
  }

  // Triggers
  m_joystick.mLeftTrigger =
      filter1d(mController.get_axis(joystick::axis::TRIGGERLEFT));
  m_joystick.mRightTrigger =
      filter1d(mController.get_axis(joystick::axis::TRIGGERRIGHT));

  // Sticks
  x = mController.get_axis(joystick::axis::LEFTX);
  y = -mController.get_axis(joystick::axis::LEFTY);
  m_joystick.mLeftStick = filter2d(x, y);

  x = mController.get_axis(joystick::axis::RIGHTX);
  y = -mController.get_axis(joystick::axis::RIGHTY);
  m_joystick.mRightStick = filter2d(x, y);

  input::mouse.hide_cursor(isHide);
}
void input_system::process_event() {

  auto e = event_system::current_event;
  switch (e.type) {
  case SDL_MOUSEWHEEL: {
    m_mouse.mScrollWheel =
        vector2(static_cast<float>(e.wheel.x), static_cast<float>(e.wheel.y));
    break;
  }
  default:
    break;
  }
}

void input::set_relative_mouse_mode(bool value) {
  input_system::set_relative_mouse_mode(value);
}

void input_system::set_relative_mouse_mode(bool _value) {
  SDL_bool set = _value ? SDL_TRUE : SDL_FALSE;
  SDL_SetRelativeMouseMode(set);

  m_mouse.mIsRelative = _value;
}

float input_system::filter1d(int _input) {
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
    retVal = math::clamp(retVal, -1.0f, 1.0f);
  }

  return retVal;
}

vector2 input_system::filter2d(int inputX, int inputY) {
  const float deadZone = 8000.0f;
  const float maxValue = 30000.0f;

  // Make into 2D vector
  vector2 dir;
  dir.x = static_cast<float>(inputX);
  dir.y = static_cast<float>(inputY);

  float length = dir.length();

  // If length < deadZone, should be no input
  if (length < deadZone) {
    dir = vector2::zero;
  } else {
    // Calculate fractional value between
    // dead zone and max value circles
    float f = (length - deadZone) / (maxValue - deadZone);
    // Clamp f between 0.0f and 1.0f
    f = math::clamp(f, 0.0f, 1.0f);
    // Normalize the vector, and then scale it to the
    // fractional value
    dir *= f / length;
  }

  return dir;
}
} // namespace sinen
