#include "luaapi.hpp"
#include <core/logger/log.hpp>

namespace sinen {
static int lLogInfo(lua_State *L) {
  lua_getglobal(L, "tostring");
  lua_pushvalue(L, 1);
  if (luaPCallLogged(L, 1, 1) != LUA_OK) {
    return 0;
  }
  const char *s = lua_tostring(L, -1);
  Log::info("{}", s);
  lua_pop(L, 1);
  return 0;
}
static int lLogVerbose(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  Log::verbose("{}", s);
  return 0;
}
static int lLogError(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  Log::error("{}", s);
  return 0;
}
static int lLogWarn(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  Log::warn("{}", s);
  return 0;
}
static int lLogCritical(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  Log::critical("{}", s);
  return 0;
}
void registerLog(lua_State *L) {
  pushSnNamed(L, "Log");
  Binding::registerFunction(L, "verbose", lLogVerbose);
  Binding::registerFunction(L, "info", lLogInfo);
  Binding::registerFunction(L, "error", lLogError);
  Binding::registerFunction(L, "warn", lLogWarn);
  Binding::registerFunction(L, "critical", lLogCritical);
  lua_pop(L, 1);
}

} // namespace sinen
