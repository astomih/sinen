#ifndef SINEN_GAME_CONTROLLER_HPP
#define SINEN_GAME_CONTROLLER_HPP
#include <cstdint>
#include <input/gamepad.hpp>
#include <memory>

namespace sinen {
class joystick {
public:
  joystick();
  ~joystick();

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
  int16_t get_axis(axis _axis);
  uint8_t get_button(GamePad::code button);

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};
} // namespace sinen
#endif // !SINEN_GAME_CONTROLLER_HPP
