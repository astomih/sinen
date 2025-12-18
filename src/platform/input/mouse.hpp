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
  static void setPosition(const glm::vec2 &pos);
  static void setPositionOnScene(const glm::vec2 &pos);
  static glm::vec2 getPosition();
  static glm::vec2 getPositionOnScene();
  static void hideCursor(bool hide);
  static glm::vec2 getScrollWheel();
  static void setRelative(bool is_relative);
  static bool isRelative();
  // For buttons
  static bool isDown(Code _button);
  static bool isPressed(Code _button);
  static bool isReleased(Code _button);
  enum class Code { LEFT = 1, MIDDLE = 2, RIGHT = 3, X1 = 4, X2 = 5 };
  using enum Code;
};
} // namespace sinen
#endif // !SINEN_MOUSE_HPP