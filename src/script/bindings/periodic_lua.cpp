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
void registerPeriodic(lua_State *L) {
  pushSnNamed(L, "Periodic");
  luau::table(L, -1).function("sin0_1", lPeriodicSin01);
  luau::table(L, -1).function("cos0_1", lPeriodicCos01);
  lua_pop(L, 1);
}
} // namespace sinen
