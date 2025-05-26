#include "../window/window_system.hpp"
#include <SDL3/SDL.h>

#include "SDL3/SDL_mouse.h"
#include "input_system.hpp"
#include "platform/input/mouse.hpp"
#include "platform/window/window.hpp"
#include <cstring>
#include <imgui_impl_sdl3.h>
#include <logic/scene/scene.hpp>
#include <platform/platform.hpp>

#include <math/math.hpp>

#include <core/logger/logger.hpp>

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

static button_state get_key_state(KeyInput::KeyCode _key) {
  if (!input_system::keyInputState.previousKeys.contains(
          static_cast<std::uint32_t>(_key))) {
    if (!input_system::keyInputState.currentKeys.contains(
            static_cast<std::uint32_t>(_key))) {
      return button_state::None;
    } else {
      return button_state::Pressed;
    }
  } else // Prev state must be 1
  {
    if (!input_system::keyInputState.currentKeys.contains(
            static_cast<std::uint32_t>(_key))) {
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
KeyInputState input_system::keyInputState = KeyInputState();
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

bool KeyInput::is_pressed(const KeyCode key) {
  return get_key_state(key) == button_state::Pressed;
}
bool KeyInput::is_down(const KeyCode key) {
  return get_key_state(key) == button_state::Held;
}
bool KeyInput::is_released(const KeyCode key) {
  return get_key_state(key) == button_state::Released;
}

void Mouse::set_position(const glm::vec2 &pos) {
  const auto half = Window::half();
  SDL_WarpMouseInWindow(WindowImpl::get_sdl_window(), half.x + pos.x,
                        half.y - pos.y);
}
void Mouse::set_position_on_scene(const glm::vec2 &pos) {
  Mouse::set_position(pos * Scene::ratio());
}
glm::vec2 Mouse::get_position() {
  glm::vec2 pos;
  SDL_GetMouseState(&pos.x, &pos.y);
  pos -= Window::half();
  pos.y *= -1.f;
  return pos;
}
glm::vec2 Mouse::get_position_on_scene() {
  return Mouse::get_position() * Scene::inv_ratio();
}

glm::vec2 Mouse::get_scroll_wheel() {
  return input_system::m_mouse.mScrollWheel;
}

void Mouse::set_relative(bool is_relative) {
  SDL_SetWindowRelativeMouseMode(WindowImpl::get_sdl_window(), is_relative);
}

bool Mouse::is_relative() {
  return SDL_GetWindowRelativeMouseMode(WindowImpl::get_sdl_window());
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
const glm::vec2 &GamePad::get_left_stick() {
  return input_system::m_joystick.mLeftStick;
}
const glm::vec2 &GamePad::get_right_stick() {
  return input_system::m_joystick.mRightStick;
}
bool GamePad::is_connected() { return input_system::m_joystick.mIsConnected; }

joystick input_system::mController;
bool input_system::initialize() {

  m_keyboard.mCurrState = SDL_GetKeyboardState(NULL);
  memcpy(m_keyboard.mPrevState.data(), m_keyboard.mCurrState,
         SDL_SCANCODE_COUNT);

  float x = 0, y = 0;
  m_mouse.mCurrButtons = SDL_GetMouseState(&x, &y);

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
  m_mouse.mScrollWheel = glm::vec2(0.f);

  // Controller
  memcpy(m_joystick.mPrevButtons, m_joystick.mCurrButtons,
         SDL_GAMEPAD_BUTTON_COUNT);
  keyInputState.previousKeys = keyInputState.currentKeys;
  keyInputState.currentKeys.clear();
}

void input_system::update() {
  // Mouse
  float x = 0, y = 0;
  m_mouse.mCurrButtons = SDL_GetMouseState(&x, &y);

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
    m_mouse.mScrollWheel = glm::vec2(static_cast<float>(event.wheel.x),
                                     static_cast<float>(event.wheel.y));
    break;
  }
  case SDL_EVENT_KEY_DOWN: {
    keyInputState.currentKeys.insert(event.key.key);
    break;
  }
  case SDL_EVENT_TEXT_INPUT: {
    break;
  }
  default:
    break;
  }
}

void input_system::set_relative_mouse_mode(bool _value) {
  bool set = _value ? true : false;
  SDL_SetWindowRelativeMouseMode(WindowImpl::get_sdl_window(), set);
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

glm::vec2 input_system::filter2d(int inputX, int inputY) {
  const float deadZone = 8000.0f;
  const float maxValue = 30000.0f;

  // Make into 2D vector
  glm::vec2 dir;
  dir.x = static_cast<float>(inputX);
  dir.y = static_cast<float>(inputY);

  float length = dir.length();

  // If length < deadZone, should be no input
  if (length < deadZone) {
    dir = glm::vec2(0.f);
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
