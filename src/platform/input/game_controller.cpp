#include <SDL3/SDL.h>
#include <SDL3/SDL_gamepad.h>

#include "game_controller.hpp"

namespace sinen {
class Joystick::Impl {
public:
  Impl() : controller(nullptr) {}
  ~Impl() = default;
  ::SDL_Gamepad *controller;
};
Joystick::Joystick() : impl(nullptr) {}

Joystick::~Joystick() = default;

bool Joystick::initialize() {
  impl = std::make_unique<Joystick::Impl>();
  impl->controller = SDL_OpenGamepad(0);
  if (impl->controller)
    return true;
  return false;
}

int16_t Joystick::getAxis(Joystick::axis _axis) {
  return ::SDL_GetGamepadAxis(this->impl->controller,
                              static_cast<SDL_GamepadAxis>(_axis));
}
uint8_t Joystick::getButton(GamePad::code button) {
  return SDL_GetGamepadButton(impl->controller,
                              static_cast<SDL_GamepadButton>(button));
}
} // namespace sinen
