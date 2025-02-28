#ifndef SINEN_MOUSE_HPP
#define SINEN_MOUSE_HPP
#include "../math/vector2.hpp"
namespace sinen {
/**
 * @brief Mouse
 *
 */
class Mouse {
public:
  enum class code;
  // For mouse position
  static void set_position(const Vector2 &pos);
  static void set_position_on_scene(const Vector2 &pos);
  static Vector2 get_position();
  static Vector2 get_position_on_scene();
  static void hide_cursor(bool hide);
  static Vector2 get_scroll_wheel();
  static bool IsRelative();
  // For buttons
  static bool is_down(code _button);
  static bool is_pressed(code _button);
  static bool is_released(code _button);
  enum class code { LEFT = 1, MIDDLE = 2, RIGHT = 3, X1 = 4, X2 = 5 };
  using enum code;
};
} // namespace sinen
#endif // !SINEN_MOUSE_HPP