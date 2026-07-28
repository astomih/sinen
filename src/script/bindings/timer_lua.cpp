#include "luaapi.hpp"
#include <core/time/time.hpp>
#include <core/time/timer.hpp>

namespace sinen {
static int lTimerNew(lua_State *L) {
  if (lua_gettop(L) >= 1 && lua_isnumber(L, 1)) {
    float t = static_cast<float>(lua_tonumber(L, 1));
    udNewOwned<Timer>(L, Timer(t));
    return 1;
  }
  udNewOwned<Timer>(L, Timer());
  return 1;
}
static int lTimerStart(lua_State *L) {
  udValue<Timer>(L, 1).start();
  return 0;
}
static int lTimerStop(lua_State *L) {
  udValue<Timer>(L, 1).stop();
  return 0;
}
static int lTimerIsStarted(lua_State *L) {
  lua_pushboolean(L, udValue<Timer>(L, 1).isStarted());
  return 1;
}
static int lTimerSetTime(lua_State *L) {
  float ms = static_cast<float>(luaL_checknumber(L, 2));
  udValue<Timer>(L, 1).setTime(ms);
  return 0;
}
static int lTimerCheck(lua_State *L) {
  lua_pushboolean(L, udValue<Timer>(L, 1).check());
  return 1;
}
void registerTimer(lua_State *L) {
  luaL_newmetatable(L, Timer::metaTableName());
  Binding::registerFunction(L, "__gc", udGc<Timer>);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "start", lTimerStart);
  Binding::registerFunction(L, "stop", lTimerStop);
  Binding::registerFunction(L, "isStarted", lTimerIsStarted);
  Binding::registerFunction(L, "setTime", lTimerSetTime);
  Binding::registerFunction(L, "check", lTimerCheck);
  lua_pop(L, 1);

  pushSnNamed(L, "Timer");
  Binding::registerFunction(L, "new", lTimerNew);
  lua_pop(L, 1);
}
} // namespace sinen
