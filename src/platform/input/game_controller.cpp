#include <SDL3/SDL.h>
#include <SDL3/SDL_gamepad.h>

#include <platform/input/game_controller.hpp>

namespace sinen {
class Joystick::Impl {
public:
  ~Impl() {
    if (controller) {
      SDL_CloseGamepad(controller);
    }
    SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
  }
  SDL_Gamepad *controller = nullptr;
  Uint64 nextScan = 0;
};
Joystick::Joystick() : impl(nullptr) {}

Joystick::~Joystick() = default;

bool Joystick::initialize() {
  if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
    return false;
  }
  impl = std::make_unique<Joystick::Impl>();
  return refresh();
}

void Joystick::shutdown() { impl.reset(); }

bool Joystick::refresh() {
  if (!impl) {
    return false;
  }
  if (impl->controller && SDL_GamepadConnected(impl->controller)) {
    return true;
  }
  if (impl->controller) {
    SDL_CloseGamepad(impl->controller);
    impl->controller = nullptr;
    impl->nextScan = 0;
  }
  const auto now = SDL_GetTicks();
  if (now < impl->nextScan) {
    return false;
  }
  impl->nextScan = now + 500;
  int count = 0;
  auto *ids = SDL_GetGamepads(&count);
  for (int i = 0; ids && i < count; ++i) {
    impl->controller = SDL_OpenGamepad(ids[i]);
    if (impl->controller) {
      break;
    }
  }
  SDL_free(ids);
  return impl->controller != nullptr;
}

const char *Joystick::getName() const {
  return impl && impl->controller ? SDL_GetGamepadName(impl->controller)
                                  : nullptr;
}

int16_t Joystick::getAxis(Joystick::axis _axis) {
  return impl && impl->controller
             ? SDL_GetGamepadAxis(impl->controller,
                                  static_cast<SDL_GamepadAxis>(_axis))
             : 0;
}
uint8_t Joystick::getButton(GamepadButton button) {
  return impl && impl->controller &&
         SDL_GetGamepadButton(impl->controller,
                              static_cast<SDL_GamepadButton>(button));
}
} // namespace sinen
