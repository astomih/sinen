#include "binding.hpp"

namespace sinen {
void Binding::beginClass(const char *metaTableName, const char *className,
                         lua_CFunction constructor) {
  pushSnNamed(L, className);
  registerFunction("new", constructor);
  lua_pop(L, 1);

  luaL_newmetatable(L, metaTableName);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
}
void Binding::registerFunction(const char *name, lua_CFunction func) {
  luaPushcfunction2(L, func);
  lua_setfield(L, -2, name);
}
void Binding::endClass() { lua_pop(L, 1); }
} // namespace sinen