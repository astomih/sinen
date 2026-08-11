#include <lua.h>
#include <lualib.h>

#include <script/sinen_lua_module.h>

static int callbackRegisteredBeforeFailure(lua_State *state) {
  lua_pushinteger(state, 7);
  return 1;
}

SINEN_LUA_MODULE_EXPORT uint32_t sinen_lua_module_get_api_version(void) {
  return SINEN_LUA_MODULE_API_VERSION;
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_open(lua_State *state) {
  lua_pushcfunction(state, callbackRegisteredBeforeFailure,
                    "native_module_failure_fixture.callback");
  lua_setglobal(state, "native_module_failure_callback");
  luaL_error(state, "intentional native module initialization failure");
  return 0;
}
