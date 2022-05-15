#include <SDL.h>
#include <SDL_gamecontroller.h>
#include <input/game_controller.hpp>

namespace nen {
class joystick::Impl {
public:
  Impl() : controller(nullptr) {}
  ~Impl() = default;
  ::SDL_GameController *controller;
};
joystick::joystick() : impl(nullptr) {}

joystick::~joystick() = default;

bool joystick::initialize() {
  impl = std::make_unique<joystick::Impl>();
  impl->controller = SDL_GameControllerOpen(0);
  if (impl->controller)
    return true;
  return false;
}

int16_t joystick::get_axis(joystick::axis _axis) {
  return ::SDL_GameControllerGetAxis(
      this->impl->controller, static_cast<SDL_GameControllerAxis>(_axis));
}
uint8_t joystick::get_button(joystick_button button) {
  return SDL_GameControllerGetButton(
      impl->controller, static_cast<SDL_GameControllerButton>(button));
}
} // namespace nen