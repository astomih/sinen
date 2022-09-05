#include "color.hpp"

namespace sinen {
/**
 * @brief HSV color
 *
 */
struct hsv {
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
  hsv(float h, float s, float v) : h(h), s(s), v(v) {}
  hsv() : hsv(0, 0, 0) {}
  hsv(const color &c) { from_color(c); }
  void from_color(const color &c);
  color to_color() const;
  operator color() const { return to_color(); }
};
} // namespace sinen