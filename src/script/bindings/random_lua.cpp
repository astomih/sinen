#include "luaapi.hpp"

#include <math/random.hpp>

namespace sinen {
namespace {
int lRandomGetRange(lua_State *L) {
  const auto min = static_cast<float>(luaL_checknumber(L, 1));
  const auto max = static_cast<float>(luaL_checknumber(L, 2));
  lua_pushnumber(L, Random::getRange(min, max));
  return 1;
}

int lRandomGetIntRange(lua_State *L) {
  const auto min = static_cast<int>(luaL_checkinteger(L, 1));
  const auto max = static_cast<int>(luaL_checkinteger(L, 2));
  lua_pushinteger(L, Random::getIntRange(min, max));
  return 1;
}
} // namespace

void registerRandom(lua_State *L) {
  pushSnNamed(L, "Random");
  lua_pushcfunction(L, lRandomGetRange, "sn.Random.getRange");
  lua_setfield(L, -2, "getRange");
  lua_pushcfunction(L, lRandomGetIntRange, "sn.Random.getIntRange");
  lua_setfield(L, -2, "getIntRange");
  lua_pop(L, 1);
}

} // namespace sinen
