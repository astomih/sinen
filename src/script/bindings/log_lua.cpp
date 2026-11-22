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
static auto lLogVerbose(const char *s) {
  Log::verbose("{}", s);
  return;
}
static auto lLogError(const char *s) {
  Log::error("{}", s);
  return;
}
static auto lLogWarn(const char *s) {
  Log::warn("{}", s);
  return;
}
static auto lLogCritical(const char *s) {
  Log::critical("{}", s);
  return;
}
void registerLog(lua_State *L) {
  pushSnNamed(L, "Log");
  luau::table(L, -1).function("verbose", lLogVerbose);
  luau::table(L, -1).raw_function("info", lLogInfo);
  luau::table(L, -1).function("error", lLogError);
  luau::table(L, -1).function("warn", lLogWarn);
  luau::table(L, -1).function("critical", lLogCritical);
  lua_pop(L, 1);
}

} // namespace sinen
