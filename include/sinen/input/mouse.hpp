#ifndef SINEN_MOUSE_HPP
#define SINEN_MOUSE_HPP
#include "../math/vector2.hpp"
namespace sinen {
/**
 * @brief Mouse
 *
 */
class mouse {
public:
  enum class code;
  // For mouse position
  static void set_position(const vector2 &pos);
  static void set_position_on_scene(const vector2 &pos);
  static const vector2 &get_position();
  static const vector2 &get_position_on_scene();
  static void hide_cursor(bool hide);
  static const vector2 &get_scroll_wheel();
  static bool IsRelative();
  // For buttons
  static bool is_down(code _button);
  static bool is_pressed(code _button);
  static bool is_released(code _button);
  enum class code { LEFT = 1, MIDDLE = 2, RIGHT = 3, X1 = 4, X2 = 5 };
};
} // namespace sinen
#endif // !SINEN_MOUSE_HPP