#include "require_alias.hpp"

#include <script/sinen_lua_module.h>

#include <lua.h>
#include <lualib.h>

#include <string>
#include <string_view>

namespace sinen {
namespace {

int requireWithSinenAlias(lua_State *state) {
  size_t moduleNameSize = 0;
  const char *moduleName = luaL_checklstring(state, 1, &moduleNameSize);
  const std::string_view moduleNameView(moduleName, moduleNameSize);
  if (moduleNameView == "@sinen") {
    lua_getglobal(state, "sn");
    return 1;
  }

  constexpr std::string_view pluginPrefix = SINEN_LUA_PLUGIN_REQUIRE_PREFIX;
  if (moduleNameView.starts_with(pluginPrefix)) {
    const std::string_view pluginName =
        moduleNameView.substr(pluginPrefix.size());
    if (pluginName.empty() || pluginName.find('/') != std::string_view::npos ||
        pluginName.find('\\') != std::string_view::npos) {
      luaL_error(state, "invalid Sinen plugin module '%s'", moduleName);
      return 0;
    }

    const std::string registryKey =
        std::string(SINEN_LUA_PLUGIN_REGISTRY_PREFIX) + std::string(pluginName);
    lua_getfield(state, LUA_REGISTRYINDEX, registryKey.c_str());
    if (lua_isnil(state, -1)) {
      luaL_error(state,
                 "Sinen plugin module '%s' is not loaded; call "
                 "sn.Script.loadPlugin(\"%s\") first",
                 moduleName, std::string(pluginName).c_str());
      return 0;
    }
    return 1;
  }

  lua_pushvalue(state, lua_upvalueindex(1));
  lua_insert(state, 1);

  const int argumentCount = lua_gettop(state) - 1;
  lua_call(state, argumentCount, LUA_MULTRET);
  return lua_gettop(state);
}

} // namespace

void installSinenRequireAlias(lua_State *state) {
  lua_getglobal(state, "require");
  if (!lua_isfunction(state, -1)) {
    lua_pop(state, 1);
    return;
  }

  lua_pushcclosure(state, requireWithSinenAlias, "require", 1);
  lua_setglobal(state, "require");
}

} // namespace sinen
