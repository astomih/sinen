#include <math/math.hpp>
#include <math/periodic.hpp>
namespace sinen {
float periodic::sin0_1(const float periodSec, const float t) {
  const auto f = math::fmod(t, periodSec);
  auto x = f / (periodSec * (1.f / (2.f * math::pi)));
  return sin(x) * 0.5f + 0.5f;
}

float periodic::cos0_1(const float periodSec, const float t) {
  const auto f = math::fmod(t, periodSec);
  const auto x = f / (periodSec * (1.f / (2.f * math::pi)));
  return cos(x) * 0.5f + 0.5f;
}
} // namespace sinen