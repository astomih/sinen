#include "slab.hpp"
#include <script/luau_bind.hpp>

SINEN_LUA_MODULE_EXPORT uint32_t sinen_lua_module_get_api_version() {
  return SINEN_LUA_MODULE_API_VERSION;
}

void registerSlab(sinen::luau::table &module) {
  module.import_type<sinen::Vec2>(sinen::Vec2::metaTableName());
  module.function("slab", &sinen::generated::run);
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_open(lua_State *state) {
  return sinen::luau::open_plugin(state, "fps_collision", registerSlab);
}
