#include <script/luaapi.hpp>
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
  luaPushcfunction2(L, lTimeSeconds);
  lua_setfield(L, -2, "seconds");
  luaPushcfunction2(L, lTimeMilli);
  lua_setfield(L, -2, "milli");
  luaPushcfunction2(L, lTimeDelta);
  lua_setfield(L, -2, "delta");
  lua_pop(L, 1);
}

} // namespace sinen
