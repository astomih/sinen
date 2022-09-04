#ifndef SINEN_PALETTE_HPP
#define SINEN_PALETTE_HPP
#include "color.hpp"
namespace sinen {
/**
 * @brief Color palette
 *
 */
struct palette {
  static const color black();
  static const color light_black();
  static const color white();
  static const color red();
  static const color green();
  static const color blue();
  static const color yellow();
  static const color light_yellow();
  static const color light_blue();
  static const color light_pink();
  static const color light_green();
};
} // namespace sinen
#endif // !SINEN_PALETTE_HPP
