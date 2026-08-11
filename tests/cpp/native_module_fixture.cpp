#include <lua.h>

#include <script/sinen_lua_module.h>

static int answer(lua_State *state) {
  lua_pushinteger(state, 42);
  return 1;
}

SINEN_LUA_MODULE_EXPORT uint32_t sinen_lua_module_get_api_version(void) {
  return SINEN_LUA_MODULE_API_VERSION;
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_open(lua_State *state) {
  lua_newtable(state);
  lua_pushcfunction(state, answer, "native_module_fixture.answer");
  lua_setfield(state, -2, "answer");
  lua_setglobal(state, "native_module_fixture");
  return 0;
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_close(lua_State *state) {
  lua_pushboolean(state, 1);
  lua_setglobal(state, "native_module_fixture_closed");
  return 0;
}
