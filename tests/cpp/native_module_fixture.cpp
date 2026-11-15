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
  return sinen::luau::open(state, [&] {
    auto module = sinen::luau::table::create(state);
    module.function("answer", [] { return 42; });
    module.class_<Counter>("Counter")
        .constructor<int>()
        .method("add", &Counter::add)
        .field("value", &Counter::value);
    module.publish("native_module_fixture")
        .publish_plugin("native_module_fixture");
  });
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_close(lua_State *state) {
  lua_pushboolean(state, 1);
  lua_setglobal(state, "native_module_fixture_closed");
  return 0;
}
