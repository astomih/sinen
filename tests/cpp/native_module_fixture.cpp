#include <script/luau_bind.hpp>

namespace {
struct Counter {
  explicit Counter(int value) : value(value) {}
  int add(int amount) { return value += amount; }
  int value;
};
} // namespace

SINEN_LUA_MODULE_EXPORT uint32_t sinen_lua_module_get_api_version(void) {
  return SINEN_LUA_MODULE_API_VERSION;
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_open(lua_State *state) {
  return sinen::luau::open_plugin(
      state, "native_module_fixture", [](sinen::luau::table &module) {
        module.function("answer", [] { return 42; });
        module.class_<Counter>("Counter")
            .constructor<int>()
            .method<&Counter::add>("add")
            .field<&Counter::value>("value");
        module.publish("native_module_fixture");
      });
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_close(lua_State *state) {
  sinen::luau::state_view{state}["native_module_fixture_closed"] = true;
  return 0;
}
