#ifndef SINEN_PERIODIC_HPP
#define SINEN_PERIODIC_HPP
#include <core/time/time.hpp>
namespace sinen {
class Periodic {
public:
  static float sineWave(const float periodSec, const float t = Time::seconds());

  static float cosWave(const float periodSec, const float t = Time::seconds());
};
} // namespace sinen
#endif // SINEN_PERIODIC_HPP