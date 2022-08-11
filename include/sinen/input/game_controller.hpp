#ifndef	SINEN_GAME_CONTROLLER_HPP
#define	SINEN_GAME_CONTROLLER_HPP
#include "game_controller_button.hpp"
#include <cstdint>
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
  uint8_t get_button(joystick_button button);

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};
} // namespace sinen
#endif // !SINEN_GAME_CONTROLLER_HPP
