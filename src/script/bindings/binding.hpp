#ifndef SINEN_BINDING_HPP
#define SINEN_BINDING_HPP

#include <Luau/Compiler.h>
#include <lua.h>
#include <lualib.h>

namespace sinen {
class Binding {
public:
  Binding(lua_State *L) : L(L) {}
  void beginClass(const char *metaTableName, const char *className,
                  lua_CFunction constructor);
  void registerFunction(const char *name, lua_CFunction func);
  void endClass();

  static void registerFunction(lua_State *L, const char *name,
                               lua_CFunction func);
  static void registerInteger(lua_State *L, const char *name,
                              lua_Integer value);
  static void registerNumber(lua_State *L, const char *name, lua_Number value);

  lua_State *L;
};
} // namespace sinen

#endif // SINEN_BINDING_HPP
