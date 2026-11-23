#include "shared_binding_fixture.hpp"
#include <script/luau_bind.hpp>

SINEN_LUA_MODULE_EXPORT uint32_t sinen_lua_module_get_api_version() {
  return SINEN_LUA_MODULE_API_VERSION;
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_open(lua_State *L) {
  return sinen::luau::open_plugin(
      L, "shared_binding_consumer", [](sinen::luau::table &module) {
        using sinen::test::SharedValue;
        module.import_type<SharedValue>("test.SharedValue", 3)
            .function("copy",
                      [](const SharedValue &value) {
                        return SharedValue(value.value);
                      })
            .function("increment", [](SharedValue &value) { ++value.value; })
            .function("read",
                      [](const SharedValue &value) { return value.value; })
            .function("array", [](std::vector<int> values) { return values; });
      });
}
