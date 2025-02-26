#include "../window/window_system.hpp"
#include <SDL3/SDL.h>

#include "input_system.hpp"
#include <cstring>
#include <imgui_impl_sdl3.h>
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
static button_state get_key_state(Keyboard::code _key_code) {
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
static button_state get_button_state(Mouse::code _button) {
  int mask = SDL_BUTTON_MASK(static_cast<int>(_button));
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
static button_state get_button_state(GamePad::code _button) {
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

bool Keyboard::is_down(Keyboard::code _key_code) {
  return get_key_state(_key_code) == button_state::Held;
}

bool Keyboard::is_pressed(Keyboard::code _key_code) {
  return get_key_state(_key_code) == button_state::Pressed;
}

bool Keyboard::is_released(Keyboard::code _key_code) {
  return get_key_state(_key_code) == button_state::Released;
}

void Mouse::set_position(const Vector2 &pos) {
  const auto half = Window::half();
  SDL_WarpMouseInWindow(WindowImpl::get_sdl_window(), half.x + pos.x,
                        half.y - pos.y);
}
void Mouse::set_position_on_scene(const Vector2 &pos) {
  auto scene_size = Scene::size();
  auto window_size = Window::size();
  auto x = pos.x * window_size.x / scene_size.x;
  auto y = pos.y * window_size.y / scene_size.y;
  const auto half = Scene::half();
  SDL_WarpMouseInWindow(WindowImpl::get_sdl_window(), half.x + x, half.y - y);
}

const Vector2 &Mouse::get_position() {

  float x = 0, y = 0;
  if (input_system::m_mouse.mIsRelative) {
    SDL_GetRelativeMouseState(&x, &y);
  } else {
    SDL_GetMouseState(&x, &y);
  }
  input_system::m_mouse.mousePosOnWindow.x = static_cast<float>(x);
  input_system::m_mouse.mousePosOnWindow.y = static_cast<float>(y);
  const auto half = Window::half();
  input_system::m_mouse.mousePosOnWindow -= half;
  input_system::m_mouse.mousePosOnWindow.y *= -1.f;

  return input_system::m_mouse.mousePosOnWindow;
}
const Vector2 &Mouse::get_position_on_scene() {

  float x = 0, y = 0;
  if (input_system::m_mouse.mIsRelative) {
    SDL_GetRelativeMouseState(&x, &y);
  } else {
    SDL_GetMouseState(&x, &y);
  }
  auto scene_size = Scene::size();
  auto window_size = Window::size();
  input_system::m_mouse.mousePosOnScene.x =
      static_cast<float>(x) * scene_size.x / window_size.x;
  input_system::m_mouse.mousePosOnScene.y =
      static_cast<float>(y) * scene_size.y / window_size.y;
  input_system::m_mouse.mousePosOnScene -= Scene::half();
  input_system::m_mouse.mousePosOnScene.y *= -1.f;

  return input_system::m_mouse.mousePosOnScene;
}

const Vector2 &Mouse::get_scroll_wheel() {
  return input_system::m_mouse.mScrollWheel;
}

void Mouse::hide_cursor(bool hide) {
  isHide = hide;
  if (hide) {
    SDL_HideCursor();
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
  } else {
    SDL_ShowCursor();
  }
}
bool Mouse::is_down(Mouse::code _button) {
  return get_button_state(_button) == button_state::Held;
}

bool Mouse::is_pressed(Mouse::code _button) {
  return get_button_state(_button) == button_state::Pressed;
}

bool Mouse::is_released(Mouse::code _button) {
  return get_button_state(_button) == button_state::Released;
}

bool GamePad::is_down(GamePad::code _button) {
  return get_button_state(_button) == button_state::Held;
}

bool GamePad::is_pressed(GamePad::code _button) {
  return get_button_state(_button) == button_state::Pressed;
}

bool GamePad::is_released(GamePad::code _button) {
  return get_button_state(_button) == button_state::Released;
}
const Vector2 &GamePad::get_left_stick() {
  return input_system::m_joystick.mLeftStick;
}
const Vector2 &GamePad::get_right_stick() {
  return input_system::m_joystick.mRightStick;
}
bool GamePad::is_connected() { return input_system::m_joystick.mIsConnected; }

joystick input_system::mController;
bool input_system::initialize() {

  m_keyboard.mCurrState = SDL_GetKeyboardState(NULL);
  memcpy(m_keyboard.mPrevState.data(), m_keyboard.mCurrState,
         SDL_SCANCODE_COUNT);

  float x = 0, y = 0;
  if (m_mouse.mIsRelative) {
    m_mouse.mCurrButtons = SDL_GetRelativeMouseState(&x, &y);
  } else {
    m_mouse.mCurrButtons = SDL_GetMouseState(&x, &y);
  }

  // Initialize controller state
  m_joystick.mIsConnected = mController.initialize();
  memset(m_joystick.mCurrButtons, 0, SDL_GAMEPAD_BUTTON_COUNT);
  memset(m_joystick.mPrevButtons, 0, SDL_GAMEPAD_BUTTON_COUNT);

  return true;
}

void input_system::shutdown() {}

void input_system::prepare_for_update() {
  // Copy current state to previous
  // Keyboard
  memcpy(m_keyboard.mPrevState.data(), m_keyboard.mCurrState,
         SDL_SCANCODE_COUNT);

  // Mouse
  m_mouse.mPrevButtons = m_mouse.mCurrButtons;
  m_mouse.mIsRelative = false;
  m_mouse.mScrollWheel = Vector2::zero;

  // Controller
  memcpy(m_joystick.mPrevButtons, m_joystick.mCurrButtons,
         SDL_GAMEPAD_BUTTON_COUNT);
}

void input_system::update() {
  // Mouse
  float x = 0, y = 0;
  if (m_mouse.mIsRelative) {
    m_mouse.mCurrButtons = SDL_GetRelativeMouseState(&x, &y);
  } else {
    m_mouse.mCurrButtons = SDL_GetMouseState(&x, &y);
  }

  // Controller
  // Buttons
  for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; i++) {
    m_joystick.mCurrButtons[i] =
        mController.get_button(static_cast<GamePad::code>(i));
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

  Mouse::hide_cursor(isHide);
}
void input_system::process_event(SDL_Event &event) {

  switch (event.type) {
  case SDL_EVENT_MOUSE_WHEEL: {
    m_mouse.mScrollWheel = Vector2(static_cast<float>(event.wheel.x),
                                   static_cast<float>(event.wheel.y));
    break;
  }
  default:
    break;
  }
}

void input_system::set_relative_mouse_mode(bool _value) {
  bool set = _value ? true : false;
  SDL_SetWindowRelativeMouseMode(WindowImpl::get_sdl_window(), set);

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
    retVal = Math::clamp(retVal, -1.0f, 1.0f);
  }

  return retVal;
}

Vector2 input_system::filter2d(int inputX, int inputY) {
  const float deadZone = 8000.0f;
  const float maxValue = 30000.0f;

  // Make into 2D vector
  Vector2 dir;
  dir.x = static_cast<float>(inputX);
  dir.y = static_cast<float>(inputY);

  float length = dir.length();

  // If length < deadZone, should be no input
  if (length < deadZone) {
    dir = Vector2::zero;
  } else {
    // Calculate fractional value between
    // dead zone and max value circles
    float f = (length - deadZone) / (maxValue - deadZone);
    // Clamp f between 0.0f and 1.0f
    f = Math::clamp(f, 0.0f, 1.0f);
    // Normalize the vector, and then scale it to the
    // fractional value
    dir *= f / length;
  }

  return dir;
}
} // namespace sinen
