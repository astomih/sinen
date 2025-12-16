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
  enum class Code;
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
  static bool IsDown(Code _button);
  static bool IsPressed(Code _button);
  static bool IsReleased(Code _button);
  enum class Code { LEFT = 1, MIDDLE = 2, RIGHT = 3, X1 = 4, X2 = 5 };
  using enum Code;
};
} // namespace sinen
#endif // !SINEN_MOUSE_HPP