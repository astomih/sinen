#include "converter.hpp"

#include <script/sinen_lua_module.h>

#include <lua.h>
#include <lualib.h>

namespace {
int convert(lua_State *state) {
  size_t sourceSize = 0;
  const char *source = luaL_checklstring(state, 1, &sourceSize);
  size_t nameSize = 0;
  const char *name = luaL_optlstring(state, 2, "<source>", &nameSize);
  const auto result =
      sinen::luau_to_cpp::convert({source, sourceSize}, {name, nameSize});
  if (!result.error.empty()) {
    lua_pushnil(state);
    lua_pushlstring(state, result.error.data(), result.error.size());
  } else {
    lua_pushlstring(state, result.cpp.data(), result.cpp.size());
    lua_pushnil(state);
  }
  return 2;
}
} // namespace

uint32_t sinen_lua_module_get_api_version() {
  return SINEN_LUA_MODULE_API_VERSION;
}

int sinen_lua_module_open(lua_State *state) {
  lua_createtable(state, 0, 1);
  lua_pushcfunction(state, convert, "convert");
  lua_setfield(state, -2, "convert");
  lua_setfield(state, LUA_REGISTRYINDEX,
               SINEN_LUA_PLUGIN_REGISTRY_PREFIX "luau_to_cpp");
  return 0;
}
