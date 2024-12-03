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
  static const Color light_black();
  static const Color white();
  static const Color red();
  static const Color green();
  static const Color blue();
  static const Color yellow();
  static const Color light_yellow();
  static const Color light_blue();
  static const Color light_pink();
  static const Color light_green();
};
} // namespace sinen
#endif // !SINEN_PALETTE_HPP
