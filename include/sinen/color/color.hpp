#ifndef SINEN_COLOR_HPP
#define SINEN_COLOR_HPP
#include "../math/vector3.hpp"
namespace sinen {
/**
 * @brief Color class
 *
 */
class Color {
public:
  /**
   * @brief Construct a new color object
   * @param red red value 0.0 - 1.0
   * @param green green value 0.0 - 1.0
   * @param blue blue value 0.0 - 1.0
   * @param alpha alpha value 0.0 - 1.0
   *
   */
  constexpr Color(const float red, const float green, const float blue,
                  const float alpha)
      : r(red), g(green), b(blue), a(alpha) {}
  /**
   * @brief Construct a new color object
   * @param value Set all value 0.0 - 1.0
   *
   */
  constexpr Color(const float value) : r(value), g(value), b(value), a(1.f) {}
  /**
   * @brief Construct a new color object
   *
   */
  Color() = default;
  // red value 0.0 - 1.0
  float r;
  // green value 0.0 - 1.0
  float g;
  // blue value 0.0 - 1.0
  float b;
  // alpha value 0.0 - 1.0
  float a;
};
} // namespace sinen
#endif // !SINEN_COLOR_HPP
