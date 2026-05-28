#include <script/luaapi.hpp>
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
  luaPushcfunction2(L, lLogVerbose);
  lua_setfield(L, -2, "verbose");
  luaPushcfunction2(L, lLogInfo);
  lua_setfield(L, -2, "info");
  luaPushcfunction2(L, lLogError);
  lua_setfield(L, -2, "error");
  luaPushcfunction2(L, lLogWarn);
  lua_setfield(L, -2, "warn");
  luaPushcfunction2(L, lLogCritical);
  lua_setfield(L, -2, "critical");
  lua_pop(L, 1);
}

} // namespace sinen
