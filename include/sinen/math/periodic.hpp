#ifndef SINEN_PERIODIC_HPP
#define SINEN_PERIODIC_HPP
#include "../time/time.hpp"
namespace sinen {
class periodic {
public:
  static float sin0_1(const float periodSec, const float t = time::seconds());

  static float cos0_1(const float periodSec, const float t = time::milli());
};
} // namespace sinen
#endif // SINEN_PERIODIC_HPP