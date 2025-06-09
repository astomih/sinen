#ifndef SINEN_GAMEPAD_HPP
#define SINEN_GAMEPAD_HPP
#include "glm/ext/vector_float2.hpp"
#include <glm/vec2.hpp>
namespace sinen {
/**
 * @brief Gamepad
 *
 */
class GamePad {
public:
  // For buttons
  enum class code;
  static bool IsDown(code j_button);
  static bool IsPressed(code j_button);
  static bool IsReleased(code j_button);
  static const glm::vec2 &GetLeftStick();
  static const glm::vec2 &GetRightStick();
  static float GetLeftTrigger();
  static float GetRightTrigger();

  static bool IsConnected();
  enum class code {
    INVALID = -1,
    A,
    B,
    X,
    Y,
    BACK,
    GUIDE,
    START,
    LEFTSTICK,
    RIGHTSTICK,
    LEFTSHOULDER,
    RIGHTSHOULDER,
    DPAD_UP,
    DPAD_DOWN,
    DPAD_LEFT,
    DPAD_RIGHT,
    MISC1, /* Xbox Series X share button, PS5 microphone button, Nintendo Switch
              Pro capture button */
    PADDLE1,  /* Xbox Elite paddle P1 */
    PADDLE2,  /* Xbox Elite paddle P3 */
    PADDLE3,  /* Xbox Elite paddle P2 */
    PADDLE4,  /* Xbox Elite paddle P4 */
    TOUCHPAD, /* PS4/PS5 touchpad button */
    NUM_GAMECONTROLLER_BUTTON
  };
  using enum code;
};
} // namespace sinen

#endif // !SINEN_GAMEPAD_HPP