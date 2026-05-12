#ifndef SINEN_GAMEPAD_HPP
#define SINEN_GAMEPAD_HPP
#include <math/vector.hpp>
namespace sinen {
/**
 * @brief Gamepad button code
 *
 */
enum class GamepadButton {
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
  COUNT,
  NUM_GAMECONTROLLER_BUTTON = COUNT
};

/**
 * @brief Gamepad
 *
 */
class GamePad {
public:
  using code = GamepadButton;
  static bool isDown(GamepadButton j_button);
  static bool isPressed(GamepadButton j_button);
  static bool isReleased(GamepadButton j_button);
  static const Vec2 &getLeftStick();
  static const Vec2 &getRightStick();
  static float getLeftTrigger();
  static float getRightTrigger();

  static bool isConnected();
  using enum GamepadButton;
};
} // namespace sinen

#endif // !SINEN_GAMEPAD_HPP
