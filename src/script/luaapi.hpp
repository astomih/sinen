#ifndef SINEN_LUAAPI_HPP
#define SINEN_LUAAPI_HPP
#include <Luau/Compiler.h>
#include <lua.h>
#include <lualib.h>

#include <core/data/array.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <core/logger/log.hpp>

#include <utility>

namespace sinen {
template <class T> struct UdBox {
  bool owned = true;
  T *ptr = nullptr;
  alignas(T) std::byte storage[sizeof(T)];
};

static void *luaLTestudata2(lua_State *L, int idx, const char *tname) {
  void *p = lua_touserdata(L, idx);
  if (!p) {
    return nullptr;
  }
  if (!lua_getmetatable(L, idx)) {
    return nullptr;
  }
  luaL_getmetatable(L, tname);
  bool ok = lua_rawequal(L, -1, -2) != 0;
  lua_pop(L, 2);
  return ok ? p : nullptr;
}

template <class T> static T &udValue(lua_State *L, int idx) {
  auto *ud =
      static_cast<UdBox<T> *>(luaL_checkudata(L, idx, T::metaTableName()));
  return *ud->ptr;
}
template <class T> static T *udValueOrNull(lua_State *L, int idx) {
  auto *ud =
      static_cast<UdBox<T> *>(luaLTestudata2(L, idx, T::metaTableName()));
  return ud ? ud->ptr : nullptr;
}
template <class T> static int udGc(lua_State *L) {
  auto *ud = static_cast<UdBox<T> *>(lua_touserdata(L, 1));
  if (ud && ud->owned && ud->ptr) {
    ud->ptr->~T();
    ud->ptr = nullptr;
  }
  return 0;
}
template <class T> static UdBox<T> *udNewOwned(lua_State *L, T value) {
  void *mem = lua_newuserdata(L, sizeof(UdBox<T>));
  auto *ud = new (mem) UdBox<T>();
  ud->owned = true;
  ud->ptr = new (ud->storage) T(std::move(value));
  luaL_getmetatable(L, T::metaTableName());
  lua_setmetatable(L, -2);
  return ud;
}
template <class T> static UdBox<T> *udNewRef(lua_State *L, T *ref) {
  void *mem = lua_newuserdata(L, sizeof(UdBox<T>));
  auto *ud = new (mem) UdBox<T>();
  ud->owned = false;
  ud->ptr = ref;
  luaL_getmetatable(L, T::metaTableName());
  lua_setmetatable(L, -2);
  return ud;
}

template <class T> static Ptr<T> &udPtr(lua_State *L, int idx) {
  return *static_cast<Ptr<T> *>(luaL_checkudata(L, idx, T::metaTableName()));
}
template <class T> static int udPtrGc(lua_State *L) {
  auto *ud = static_cast<Ptr<T> *>(lua_touserdata(L, 1));
  ud->~Ptr<T>();
  return 0;
}
template <class T> static void udPushPtr(lua_State *L, Ptr<T> value) {
  void *mem = lua_newuserdata(L, sizeof(Ptr<T>));
  new (mem) Ptr<T>(std::move(value));
  luaL_getmetatable(L, T::metaTableName());
  lua_setmetatable(L, -2);
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

static int luaPCallLogged(lua_State *L, int nargs, int nresults) {
  if (lua_pcall(L, nargs, nresults, 0) == LUA_OK) {
    return LUA_OK;
  }
  const char *msg = lua_tostring(L, -1);
  LogF::error("[lua error] {}", msg ? msg : "(unknown error)");
  lua_pop(L, 1);
  return LUA_ERRRUN;
}
void luaPushcfunction2(lua_State *L, lua_CFunction f);
int luaLError2(lua_State *L, const char *fmt, ...);
int luaLRef2(lua_State *L, int idx);
void luaLUnref2(lua_State *L, int idx, int r);
using TablePair = Array<std::pair<String, String>>;
String convert(StringView name, const TablePair &p, bool isReturn);
String toStringTrim(double value);
} // namespace sinen
#endif // SINEN_LUAAPI_HPP