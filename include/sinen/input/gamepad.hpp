#ifndef SINEN_GAMEPAD_HPP
#define SINEN_GAMEPAD_HPP
#include "../math/vector2.hpp"
namespace sinen {
/**
 * @brief Gamepad
 *
 */
class gamepad {
public:
  // For buttons
  enum class code;
  static bool is_down(code j_button);
  static bool is_pressed(code j_button);
  static bool is_released(code j_button);
  static const vector2 &get_left_stick();
  static const vector2 &get_right_stick();
  static float get_left_trigger();
  static float get_right_trigger();

  static bool is_connected();
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