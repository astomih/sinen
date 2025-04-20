#include "color.hpp"

namespace sinen {
/**
 * @brief HSV color
 *
 */
struct Hsv {
  // Hue
  float h;
  // Saturation
  float s;
  // Value(Brightness)
  float v;
  /**
   * @brief Construct a new hsv object
   *
   * @param h Hue
   * @param s Saturation
   * @param v Value(Brightness)
   */
  Hsv(float h, float s, float v) : h(h), s(s), v(v) {}
  Hsv() : Hsv(0, 0, 0) {}
  Hsv(const Color &c) { from_color(c); }
  void from_color(const Color &c);
  Color to_color() const;
  operator Color() const { return to_color(); }
};
} // namespace sinen