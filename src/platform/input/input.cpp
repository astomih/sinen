#include "../window/window_system.hpp"
#include <SDL3/SDL.h>

#include "../../graphics/graphics_system.hpp"
#include "SDL3/SDL_mouse.h"
#include "input_system.hpp"
#include "platform/input/mouse.hpp"
#include "platform/window/window.hpp"
#include <cstring>
#include <imgui_impl_sdl3.h>
#include <platform/platform.hpp>

#include <math/math.hpp>

#include <core/logger/logger.hpp>

namespace sinen {
bool isHide = false;
SDL_Cursor *gCursor = nullptr;
/**
 * @brief Button state
 *
 */
enum class button_state { None, Pressed, Released, Held };
static button_state getKeyState(Keyboard::Code _key_code) {
  if (InputSystem::mKeyboard.mPrevState[static_cast<int>(_key_code)] == 0) {
    if (InputSystem::mKeyboard.mCurrState[static_cast<int>(_key_code)] == 0) {
      return button_state::None;
    } else {
      return button_state::Pressed;
    }
  } else // Prev state must be 1
  {
    if (InputSystem::mKeyboard.mCurrState[static_cast<int>(_key_code)] == 0) {
      return button_state::Released;
    } else {
      return button_state::Held;
    }
  }
}

static button_state getKeyState(KeyInput::KeyCode _key) {
  if (!InputSystem::keyInputState.previousKeys.contains(
          static_cast<std::uint32_t>(_key))) {
    if (!InputSystem::keyInputState.currentKeys.contains(
            static_cast<std::uint32_t>(_key))) {
      return button_state::None;
    } else {
      return button_state::Pressed;
    }
  } else // Prev state must be 1
  {
    if (!InputSystem::keyInputState.currentKeys.contains(
            static_cast<std::uint32_t>(_key))) {
      return button_state::Released;
    } else {
      return button_state::Held;
    }
  }
}

static button_state getButtonState(Mouse::Code _button) {
  int mask = SDL_BUTTON_MASK(static_cast<int>(_button));
  if ((mask & InputSystem::mMouse.mPrevButtons) == 0) {
    if ((mask & InputSystem::mMouse.mCurrButtons) == 0) {
      return button_state::None;
    } else {
      return button_state::Pressed;
    }
  } else {
    if ((mask & InputSystem::mMouse.mCurrButtons) == 0) {
      return button_state::Released;
    } else {
      return button_state::Held;
    }
  }
}
static button_state getButtonState(GamePad::code _button) {
  if (InputSystem::mJoystick.mPrevButtons[static_cast<int>(_button)] == 0) {
    if (InputSystem::mJoystick.mCurrButtons[static_cast<int>(_button)] == 0) {
      return button_state::None;
    } else {
      return button_state::Pressed;
    }
  } else // Prev state must be 1
  {
    if (InputSystem::mJoystick.mCurrButtons[static_cast<int>(_button)] == 0) {
      return button_state::Released;
    } else {
      return button_state::Held;
    }
  }
}

KeyboardStateImpl InputSystem::mKeyboard = KeyboardStateImpl();
KeyInputState InputSystem::keyInputState = KeyInputState();
MouseStateImpl InputSystem::mMouse = MouseStateImpl();
JoystickStateImpl InputSystem::mJoystick = JoystickStateImpl();

bool Keyboard::isDown(Keyboard::Code _key_code) {
  return getKeyState(_key_code) == button_state::Held;
}

bool Keyboard::isPressed(Keyboard::Code _key_code) {
  return getKeyState(_key_code) == button_state::Pressed;
}

bool Keyboard::isReleased(Keyboard::Code _key_code) {
  return getKeyState(_key_code) == button_state::Released;
}

bool KeyInput::isPressed(const KeyCode key) {
  return getKeyState(key) == button_state::Pressed;
}
bool KeyInput::isDown(const KeyCode key) {
  return getKeyState(key) == button_state::Held;
}
bool KeyInput::isReleased(const KeyCode key) {
  return getKeyState(key) == button_state::Released;
}

void Mouse::setPosition(const glm::vec2 &pos) {
  const auto half = Window::half();
  SDL_WarpMouseInWindow(WindowSystem::getSdlWindow(), half.x + pos.x,
                        half.y - pos.y);
}
void Mouse::setPositionOnScene(const glm::vec2 &pos) {
  Mouse::setPosition(pos * GraphicsSystem::getCamera2D().windowRatio());
}
glm::vec2 Mouse::getPosition() {
  glm::vec2 pos;
  SDL_GetMouseState(&pos.x, &pos.y);
  pos -= Window::half();
  pos.y *= -1.f;
  return pos;
}
glm::vec2 Mouse::getPositionOnScene() {
  return Mouse::getPosition() * GraphicsSystem::getCamera2D().invWindowRatio();
}

glm::vec2 Mouse::getScrollWheel() { return InputSystem::mMouse.mScrollWheel; }

void Mouse::setRelative(bool is_relative) {
  SDL_SetWindowRelativeMouseMode(WindowSystem::getSdlWindow(), is_relative);
}

bool Mouse::isRelative() {
  return SDL_GetWindowRelativeMouseMode(WindowSystem::getSdlWindow());
}

void Mouse::hideCursor(bool hide) {
  isHide = hide;
  if (hide) {
    SDL_HideCursor();
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
  } else {
    SDL_ShowCursor();
  }
}
bool Mouse::isDown(Mouse::Code _button) {
  return getButtonState(_button) == button_state::Held;
}

bool Mouse::isPressed(Mouse::Code _button) {
  return getButtonState(_button) == button_state::Pressed;
}

bool Mouse::isReleased(Mouse::Code _button) {
  return getButtonState(_button) == button_state::Released;
}

bool GamePad::isDown(GamePad::code _button) {
  return getButtonState(_button) == button_state::Held;
}

bool GamePad::isPressed(GamePad::code _button) {
  return getButtonState(_button) == button_state::Pressed;
}

bool GamePad::isReleased(GamePad::code _button) {
  return getButtonState(_button) == button_state::Released;
}
const glm::vec2 &GamePad::getLeftStick() {
  return InputSystem::mJoystick.mLeftStick;
}
const glm::vec2 &GamePad::getRightStick() {
  return InputSystem::mJoystick.mRightStick;
}
bool GamePad::isConnected() { return InputSystem::mJoystick.mIsConnected; }

Joystick InputSystem::mController;
bool InputSystem::initialize() {

  mKeyboard.mCurrState = SDL_GetKeyboardState(NULL);
  memcpy(mKeyboard.mPrevState.data(), mKeyboard.mCurrState, SDL_SCANCODE_COUNT);

  float x = 0, y = 0;
  mMouse.mCurrButtons = SDL_GetMouseState(&x, &y);

  // Initialize controller state
  mJoystick.mIsConnected = mController.initialize();
  memset(mJoystick.mCurrButtons, 0, SDL_GAMEPAD_BUTTON_COUNT);
  memset(mJoystick.mPrevButtons, 0, SDL_GAMEPAD_BUTTON_COUNT);

  return true;
}

void InputSystem::shutdown() {}

void InputSystem::prepareForUpdate() {
  // Copy current state to previous
  // Keyboard
  memcpy(mKeyboard.mPrevState.data(), mKeyboard.mCurrState, SDL_SCANCODE_COUNT);

  // Mouse
  mMouse.mPrevButtons = mMouse.mCurrButtons;
  mMouse.mScrollWheel = glm::vec2(0.f);

  // Controller
  memcpy(mJoystick.mPrevButtons, mJoystick.mCurrButtons,
         SDL_GAMEPAD_BUTTON_COUNT);
  keyInputState.previousKeys = keyInputState.currentKeys;
  keyInputState.currentKeys.clear();
}

void InputSystem::update() {
  // Mouse
  float x = 0, y = 0;
  mMouse.mCurrButtons = SDL_GetMouseState(&x, &y);

  // Controller
  // Buttons
  for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; i++) {
    mJoystick.mCurrButtons[i] =
        mController.getButton(static_cast<GamePad::code>(i));
  }

  // Triggers
  mJoystick.mLeftTrigger =
      filter1d(mController.getAxis(Joystick::axis::TRIGGERLEFT));
  mJoystick.mRightTrigger =
      filter1d(mController.getAxis(Joystick::axis::TRIGGERRIGHT));

  // Sticks
  x = mController.getAxis(Joystick::axis::LEFTX);
  y = -mController.getAxis(Joystick::axis::LEFTY);
  mJoystick.mLeftStick = filter2d(x, y);

  x = mController.getAxis(Joystick::axis::RIGHTX);
  y = -mController.getAxis(Joystick::axis::RIGHTY);
  mJoystick.mRightStick = filter2d(x, y);

  Mouse::hideCursor(isHide);
}
void InputSystem::processEvent(SDL_Event &event) {

  switch (event.type) {
  case SDL_EVENT_MOUSE_WHEEL: {
    mMouse.mScrollWheel = glm::vec2(static_cast<float>(event.wheel.x),
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

void InputSystem::setRelativeMouseMode(bool _value) {
  bool set = _value ? true : false;
  SDL_SetWindowRelativeMouseMode(WindowSystem::getSdlWindow(), set);
}

float InputSystem::filter1d(int _input) {
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

glm::vec2 InputSystem::filter2d(int inputX, int inputY) {
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
