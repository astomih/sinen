#include <math/math.hpp>
#include <math/periodic.hpp>
namespace sinen {
float Periodic::sineWave(const float periodSec, const float t) {
  const auto f = Math::fmod(t, periodSec);
  auto x = f / (periodSec * (1.f / (2.f * Math::pi)));
  return sin(x) * 0.5f + 0.5f;
}

float Periodic::cosWave(const float periodSec, const float t) {
  const auto f = Math::fmod(t, periodSec);
  const auto x = f / (periodSec * (1.f / (2.f * Math::pi)));
  return cos(x) * 0.5f + 0.5f;
}
} // namespace sinen