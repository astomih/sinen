#include "luaapi.hpp"
#include <math/math.hpp>
#include <math/periodic.hpp>

namespace sinen {
static auto lPeriodicSin01(float period, float t) {
  return Periodic::sineWave(period, t);
}
static auto lPeriodicCos01(float period, float t) {
  return Periodic::cosWave(period, t);
}
void registerPeriodic(luau::table &sn) {
  auto periodic = sn.get_or_create_table("Periodic");
  periodic.function<lPeriodicSin01>("sin0_1");
  periodic.function<lPeriodicCos01>("cos0_1");
}
} // namespace sinen
