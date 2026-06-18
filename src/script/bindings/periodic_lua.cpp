#include "luaapi.hpp"
#include <math/math.hpp>
#include <math/periodic.hpp>

namespace sinen {
static int lPeriodicSin01(lua_State *L) {
  float period = static_cast<float>(luaL_checknumber(L, 1));
  float t = static_cast<float>(luaL_checknumber(L, 2));
  lua_pushnumber(L, Periodic::sineWave(period, t));
  return 1;
}
static int lPeriodicCos01(lua_State *L) {
  float period = static_cast<float>(luaL_checknumber(L, 1));
  float t = static_cast<float>(luaL_checknumber(L, 2));
  lua_pushnumber(L, Periodic::cosWave(period, t));
  return 1;
}
void registerPeriodic(lua_State *L) {
  pushSnNamed(L, "Periodic");
  luaPushcfunction2(L, lPeriodicSin01);
  lua_setfield(L, -2, "sin0_1");
  luaPushcfunction2(L, lPeriodicCos01);
  lua_setfield(L, -2, "cos0_1");
  lua_pop(L, 1);
}
} // namespace sinen
