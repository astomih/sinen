#include "luaapi.hpp"
#include <math/random.hpp>
#include <math/vector.hpp>

namespace sinen {
static int lRandomGetRange(lua_State *L) {
  float min = static_cast<float>(luaL_checknumber(L, 1));
  float max = static_cast<float>(luaL_checknumber(L, 2));
  lua_pushnumber(L, Random::getRange(min, max));
  return 1;
}
static int lRandomGetIntRange(lua_State *L) {
  int min = static_cast<int>(luaL_checkinteger(L, 1));
  int max = static_cast<int>(luaL_checkinteger(L, 2));
  lua_pushinteger(L, Random::getIntRange(min, max));
  return 1;
}
void registerRandom(lua_State *L) {
  pushSnNamed(L, "Random");
  luaPushcfunction2(L, lRandomGetRange);
  lua_setfield(L, -2, "getRange");
  luaPushcfunction2(L, lRandomGetIntRange);
  lua_setfield(L, -2, "getIntRange");
  lua_pop(L, 1);
}

} // namespace sinen
