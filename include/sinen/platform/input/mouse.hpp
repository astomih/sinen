#ifndef SINEN_MOUSE_HPP
#define SINEN_MOUSE_HPP
#include <glm/vec2.hpp>
namespace sinen {
/**
 * @brief Mouse
 *
 */
class Mouse {
public:
  enum class code;
  // For mouse position
  static void SetPosition(const glm::vec2 &pos);
  static void SetPositionOnScene(const glm::vec2 &pos);
  static glm::vec2 GetPosition();
  static glm::vec2 GetPositionOnScene();
  static void HideCursor(bool hide);
  static glm::vec2 GetScrollWheel();
  static void SetRelative(bool is_relative);
  static bool IsRelative();
  // For buttons
  static bool IsDown(code _button);
  static bool IsPressed(code _button);
  static bool IsReleased(code _button);
  enum class code { LEFT = 1, MIDDLE = 2, RIGHT = 3, X1 = 4, X2 = 5 };
  using enum code;
};
} // namespace sinen
#endif // !SINEN_MOUSE_HPP