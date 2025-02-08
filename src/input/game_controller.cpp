#include <SDL.h>
#include <SDL_gamepad.h>

#include "game_controller.hpp"

namespace sinen {
class joystick::Impl {
public:
  Impl() : controller(nullptr) {}
  ~Impl() = default;
  ::SDL_Gamepad *controller;
};
joystick::joystick() : impl(nullptr) {}

joystick::~joystick() = default;

bool joystick::initialize() {
  impl = std::make_unique<joystick::Impl>();
  impl->controller = SDL_OpenGamepad(0);
  if (impl->controller)
    return true;
  return false;
}

int16_t joystick::get_axis(joystick::axis _axis) {
  return ::SDL_GetGamepadAxis(this->impl->controller,
                              static_cast<SDL_GamepadAxis>(_axis));
}
uint8_t joystick::get_button(GamePad::code button) {
  return SDL_GetGamepadButton(impl->controller,
                              static_cast<SDL_GamepadButton>(button));
}
} // namespace sinen
