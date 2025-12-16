#ifndef SINEN_PERIODIC_HPP
#define SINEN_PERIODIC_HPP
#include <core/time/time.hpp>
namespace sinen {
class Periodic {
public:
  static float sin0_1(const float periodSec, const float t = Time::seconds());

  static float cos0_1(const float periodSec, const float t = Time::milli());
};
} // namespace sinen
#endif // SINEN_PERIODIC_HPP