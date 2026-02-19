#include <script/script.hpp>
#include <script/luaapi.hpp>

namespace sinen {
static int lScriptLoad(lua_State *L) {
  int n = lua_gettop(L);
  const char *filePath = luaL_checkstring(L, 1);
  if (n >= 2) {
    const char *baseDirPath = luaL_checkstring(L, 2);
    Script::load(StringView(filePath), StringView(baseDirPath));
    return 0;
  }
  Script::load(StringView(filePath));
  return 0;
}
void registerScript(lua_State *L) {
  pushSnNamed(L, "Script");
  luaPushcfunction2(L, lScriptLoad);
  lua_setfield(L, -2, "load");
  lua_pop(L, 1);
}
} // namespace sinen
