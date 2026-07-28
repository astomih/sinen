#include "luaapi.hpp"
#include <core/time/time.hpp>

namespace sinen {
static int lTimeSeconds(lua_State *L) {
  lua_pushnumber(L, Time::seconds());
  return 1;
}
static int lTimeMilli(lua_State *L) {
  lua_pushinteger(L, static_cast<lua_Integer>(Time::milli()));
  return 1;
}
static int lTimeDelta(lua_State *L) {
  lua_pushnumber(L, Time::deltaTime());
  return 1;
}
void registerTime(lua_State *L) {
  pushSnNamed(L, "Time");
  Binding::registerFunction(L, "seconds", lTimeSeconds);
  Binding::registerFunction(L, "milli", lTimeMilli);
  Binding::registerFunction(L, "delta", lTimeDelta);
  lua_pop(L, 1);
}

} // namespace sinen
