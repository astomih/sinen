#ifndef	SINEN_GAME_CONTROLLER_BUTTON_HPP
#define	SINEN_GAME_CONTROLLER_BUTTON_HPP

namespace sinen {
enum class joystick_button {
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
  MISC1,   /* Xbox Series X share button, PS5 microphone button, Nintendo Switch
              Pro capture button */
  PADDLE1, /* Xbox Elite paddle P1 */
  PADDLE2, /* Xbox Elite paddle P3 */
  PADDLE3, /* Xbox Elite paddle P2 */
  PADDLE4, /* Xbox Elite paddle P4 */
  TOUCHPAD, /* PS4/PS5 touchpad button */
  NUM_GAMECONTROLLER_BUTTON
};
}
#endif // !SINEN_GAME_CONTROLLER_BUTTON_HPP
