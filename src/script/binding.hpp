#ifndef SINEN_BINDING_HPP
#define SINEN_BINDING_HPP

#include "luaapi.hpp"

namespace sinen {
class Binding {
public:
  Binding(lua_State *L) : L(L) {}
  void beginClass(const char *metaTableName, lua_CFunction gc,
                  const char *className, lua_CFunction constructor);
  void registerFunction(const char *name, lua_CFunction func);
  void endClass();
  lua_State *L;
};
} // namespace sinen

#endif // SINEN_BINDING_HPP