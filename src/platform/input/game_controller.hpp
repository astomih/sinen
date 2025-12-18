#ifndef SINEN_GAME_CONTROLLER_HPP
#define SINEN_GAME_CONTROLLER_HPP
#include <cstdint>
#include <memory>
#include <platform/input/gamepad.hpp>

namespace sinen {
class Joystick {
public:
  Joystick();
  ~Joystick();

  enum class axis {
    INVALID = -1,
    LEFTX,
    LEFTY,
    RIGHTX,
    RIGHTY,
    TRIGGERLEFT,
    TRIGGERRIGHT,
    MAX
  };

  bool initialize();
  int16_t getAxis(axis _axis);
  uint8_t getButton(GamePad::code button);

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};
} // namespace sinen
#endif // !SINEN_GAME_CONTROLLER_HPP
