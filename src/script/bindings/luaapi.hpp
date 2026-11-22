#ifndef SINEN_LUAAPI_HPP
#define SINEN_LUAAPI_HPP
#include <Luau/Compiler.h>
#include <lua.h>
#include <lualib.h>

#include <core/data/array.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <core/data/table_string.hpp>
#include <core/logger/log.hpp>

#include <utility>

#include <script/luau_bind.hpp>

// The engine's allocator-aware string uses the same Luau representation.
namespace sinen::luau {
template <> struct stack<sinen::String> : stack<std::string_view> {
  static sinen::String get(lua_State *L, int index) {
    return sinen::String(stack<std::string_view>::get(L, index));
  }
};
} // namespace sinen::luau

namespace sinen {
// Raw C API handlers share the typed binder's userdata representation.
template <class T> static T &udValue(lua_State *L, int idx) {
  return luau::stack<std::reference_wrapper<T>>::get(L, idx).get();
}
template <class T> static T *udValueOrNull(lua_State *L, int idx) {
  return luau::stack<T>::matches(L, idx) ? &udValue<T>(L, idx) : nullptr;
}
template <class T> static void udNewOwned(lua_State *L, T value) {
  luau::stack<T>::push(L, std::move(value));
}
template <class T> static void udNewRef(lua_State *L, T *ref) {
  luau::stack<std::reference_wrapper<T>>::push(L, std::ref(*ref));
}
template <class T> static Ptr<T> &udPtr(lua_State *L, int idx) {
  return udValue<Ptr<T>>(L, idx);
}
template <class T> static void udPushPtr(lua_State *L, Ptr<T> value) {
  luau::stack<Ptr<T>>::push(L, std::move(value));
}

static void *luaLTestudata2(lua_State *L, int idx, const char *name) {
  if (lua_type(L, idx) != LUA_TUSERDATA || !lua_getmetatable(L, idx))
    return nullptr;
  luaL_getmetatable(L, name);
  const bool matches = lua_rawequal(L, -1, -2) != 0;
  lua_pop(L, 2);
  return matches ? lua_touserdata(L, idx) : nullptr;
}

static void pushSn(lua_State *L) { lua_getglobal(L, "sn"); }
static void ensureSn(lua_State *L) {
  lua_getglobal(L, "sn");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "sn");
  }
}
void pushSnNamed(lua_State *L, const char *name);

// Named metatables remain available to the few raw handlers that inspect
// dynamic argument types. Allocation, type identity and destruction belong
// exclusively to luau_bind.
template <class T>
static luau::class_builder<T> engineClass(lua_State *L, const char *metaName) {
  ensureSn(L);
  luau::table sn(L, -1);
  lua_pop(L, 1);
  const char *name = metaName;
  if (std::string_view(name).substr(0, 3) == "sn.")
    name += 3;
  auto binding = sn.class_<T>(name);
  binding.metatable().push();
  lua_setfield(L, LUA_REGISTRYINDEX, metaName);
  return binding;
}

template <class T>
static void pushClassMetatable(lua_State *L, const char *metaName) {
  engineClass<T>(L, metaName).metatable().push();
}

static int luaPCallLogged(lua_State *L, int nargs, int nresults) {
  if (lua_pcall(L, nargs, nresults, 0) == LUA_OK) {
    return LUA_OK;
  }
  const char *msg = lua_tostring(L, -1);
  Log::error("[lua error] {}", msg ? msg : "(unknown error)");
  lua_pop(L, 1);
  return LUA_ERRRUN;
}
int luaLError2(lua_State *L, const char *fmt, ...);
int luaLRef2(lua_State *L, int idx);
void luaLUnref2(lua_State *L, int idx, int r);
lua_State *getGlobalLua();
} // namespace sinen
#endif // SINEN_LUAAPI_HPP
