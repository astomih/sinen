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
  static void set_position(const glm::vec2 &pos);
  static void set_position_on_scene(const glm::vec2 &pos);
  static glm::vec2 get_position();
  static glm::vec2 get_position_on_scene();
  static void hide_cursor(bool hide);
  static glm::vec2 get_scroll_wheel();
  static void set_relative(bool is_relative);
  static bool is_relative();
  // For buttons
  static bool is_down(code _button);
  static bool is_pressed(code _button);
  static bool is_released(code _button);
  enum class code { LEFT = 1, MIDDLE = 2, RIGHT = 3, X1 = 4, X2 = 5 };
  using enum code;
};
} // namespace sinen
#endif // !SINEN_MOUSE_HPP