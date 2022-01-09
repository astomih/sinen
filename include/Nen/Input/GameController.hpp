#pragma once
#include "GameControllerButton.hpp"
#include <cstdint>
#include <memory>

namespace nen {
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

  bool Initialize();
  int16_t GetAxis(axis _axis);
  uint8_t GetButton(joystick_button button);

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};
} // namespace nen
