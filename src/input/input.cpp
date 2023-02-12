#include "../window/window_system.hpp"
#include <SDL.h>

#include "input_system.hpp"
#include <cstring>
#include <imgui_impl_sdl.h>
#include <scene/scene.hpp>
#include <window/window.hpp>

#include <input/gamepad.hpp>
#include <input/keyboard.hpp>
#include <input/mouse.hpp>

namespace sinen {
bool isHide = false;
SDL_Cursor *g_cursor = nullptr;
/**
 * @brief Button state
 *
 */
enum class button_state { None, Pressed, Released, Held };
static button_state get_key_state(keyboard::code _key_code) {
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
static button_state get_button_state(mouse::code _button) {
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
static button_state get_button_state(gamepad::code _button) {
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

keyboard_state_impl input_system::m_keyboard = keyboard_state_impl();
mouse_state_impl input_system::m_mouse = mouse_state_impl();
joystick_state_impl input_system::m_joystick = joystick_state_impl();

bool keyboard::is_down(keyboard::code _key_code) {
  return get_key_state(_key_code) == button_state::Held;
}

bool keyboard::is_pressed(keyboard::code _key_code) {
  return get_key_state(_key_code) == button_state::Pressed;
}

bool keyboard::is_released(keyboard::code _key_code) {
  return get_key_state(_key_code) == button_state::Released;
}

void mouse::set_position(const vector2 &pos) {
  SDL_WarpMouseInWindow(window_system::get_sdl_window(), pos.x, pos.y);
}
void mouse::set_position_on_scene(const vector2 &pos) {
  auto scene_size = scene::size();
  auto window_size = window::size();
  auto x = pos.x * window_size.x / scene_size.x;
  auto y = pos.y * window_size.y / scene_size.y;
  SDL_WarpMouseInWindow(window_system::get_sdl_window(), x, y);
}

const vector2 &mouse::get_position() {

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
const vector2 &mouse::get_position_on_scene() {

  int x = 0, y = 0;
  if (input_system::m_mouse.mIsRelative) {
    SDL_GetRelativeMouseState(&x, &y);
  } else {
    SDL_GetMouseState(&x, &y);
  }
  auto scene_size = scene::size();
  auto window_size = window::size();
  input_system::m_mouse.mMousePos.x =
      static_cast<float>(x) * scene_size.x / window_size.x;
  input_system::m_mouse.mMousePos.y =
      static_cast<float>(y) * scene_size.y / window_size.y;
  return input_system::m_mouse.mMousePos;
}

const vector2 &mouse::get_scroll_wheel() {
  return input_system::m_mouse.mScrollWheel;
}

void mouse::hide_cursor(bool hide) {
  isHide = hide;
  if (hide) {
    SDL_ShowCursor(SDL_DISABLE);
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
  } else {
    SDL_ShowCursor(SDL_ENABLE);
  }
}
bool mouse::is_down(mouse::code _button) {
  return get_button_state(_button) == button_state::Held;
}

bool mouse::is_pressed(mouse::code _button) {
  return get_button_state(_button) == button_state::Pressed;
}

bool mouse::is_released(mouse::code _button) {
  return get_button_state(_button) == button_state::Released;
}

bool gamepad::is_down(gamepad::code _button) {
  return get_button_state(_button) == button_state::Held;
}

bool gamepad::is_pressed(gamepad::code _button) {
  return get_button_state(_button) == button_state::Pressed;
}

bool gamepad::is_released(gamepad::code _button) {
  return get_button_state(_button) == button_state::Released;
}
const vector2 &gamepad::get_left_stick() {
  return input_system::m_joystick.mLeftStick;
}
const vector2 &gamepad::get_right_stick() {
  return input_system::m_joystick.mRightStick;
}
bool gamepad::is_connected() { return input_system::m_joystick.mIsConnected; }

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
        mController.get_button(static_cast<gamepad::code>(i));
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

  mouse::hide_cursor(isHide);
}
void input_system::process_event(SDL_Event &event) {

  switch (event.type) {
  case SDL_MOUSEWHEEL: {
    m_mouse.mScrollWheel = vector2(static_cast<float>(event.wheel.x),
                                   static_cast<float>(event.wheel.y));
    break;
  }
  default:
    break;
  }
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
