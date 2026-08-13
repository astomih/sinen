#include "binding.hpp"

namespace sinen {
void Binding::registerFunction(lua_State *L, const char *name,
                               lua_CFunction func) {
  lua_pushcfunction(L, func, name);
  lua_setfield(L, -2, name);
}

void Binding::registerInteger(lua_State *L, const char *name,
                              lua_Integer value) {
  lua_pushinteger(L, value);
  lua_setfield(L, -2, name);
}

void Binding::registerNumber(lua_State *L, const char *name, lua_Number value) {
  lua_pushnumber(L, value);
  lua_setfield(L, -2, name);
}
} // namespace sinen
