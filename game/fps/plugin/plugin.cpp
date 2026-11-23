#include "slab.generated.hpp"

#include <script/sinen_lua_module.h>

#include <lua.h>
#include <lualib.h>

namespace {
int slab(lua_State *state) {
  const double origin = luaL_checknumber(state, 1);
  const double direction = luaL_checknumber(state, 2);
  const double lo = luaL_checknumber(state, 3);
  const double hi = luaL_checknumber(state, 4);
  const auto [entry, exit] = sinen::generated::run(origin, direction, lo, hi);
  lua_pushnumber(state, entry);
  lua_pushnumber(state, exit);
  return 2;
}
} // namespace

uint32_t sinen_lua_module_get_api_version() {
  return SINEN_LUA_MODULE_API_VERSION;
}

int sinen_lua_module_open(lua_State *state) {
  lua_createtable(state, 0, 1);
  lua_pushcfunction(state, slab, "slab");
  lua_setfield(state, -2, "slab");
  lua_setfield(state, LUA_REGISTRYINDEX,
               SINEN_LUA_PLUGIN_REGISTRY_PREFIX "fps_collision");
  return 0;
}
