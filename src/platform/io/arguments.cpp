#include <platform/io/arguments.hpp>
#include <script/luaapi.hpp>

namespace sinen {
int Arguments::getArgc() { return argc; }
Array<String> Arguments::getArgv() { return argv; }

static int lArgumentsGetArgc(lua_State *L) {
  lua_pushinteger(L, Arguments::getArgc());
  return 1;
}
static int lArgumentsGetArgv(lua_State *L) {
  auto argv = Arguments::getArgv();
  lua_newtable(L);
  for (size_t i = 0; i < argv.size(); ++i) {
    lua_pushlstring(L, argv[i].data(), argv[i].size());
    lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
  }
  return 1;
}
void registerArguments(lua_State *L) {
  pushSnNamed(L, "Arguments");
  luaPushcfunction2(L, lArgumentsGetArgc);
  lua_setfield(L, -2, "getArgc");
  luaPushcfunction2(L, lArgumentsGetArgv);
  lua_setfield(L, -2, "getArgv");
  lua_pop(L, 1);
}
} // namespace sinen
