#ifndef SINEN_PALETTE_HPP
#define SINEN_PALETTE_HPP
#include "color.hpp"
namespace sinen {
/**
 * @brief Color palette
 *
 */
struct Palette {
  static const Color black();
  static const Color gray();
  static const Color white();
  static const Color red();
  static const Color green();
  static const Color blue();
  static const Color yellow();
  static const Color lightYellow();
  static const Color lightBlue();
  static const Color lightPink();
  static const Color lightGreen();
};
} // namespace sinen
#endif // !SINEN_PALETTE_HPP
