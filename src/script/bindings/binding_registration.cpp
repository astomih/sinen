#include "binding.hpp"

#include <LuaBridge/LuaBridge.h>

namespace sinen {
namespace {
template <class T> void registerValue(lua_State *L, const char *name, T value) {
  if (!luabridge::push(L, value)) {
    luaL_error(L, "failed to register binding '%s'", name);
    return;
  }
  lua_setfield(L, -2, name);
}
} // namespace

void Binding::registerFunction(lua_State *L, const char *name,
                               lua_CFunction func) {
  registerValue(L, name, func);
}

void Binding::registerInteger(lua_State *L, const char *name,
                              lua_Integer value) {
  registerValue(L, name, value);
}

void Binding::registerNumber(lua_State *L, const char *name, lua_Number value) {
  registerValue(L, name, value);
}
} // namespace sinen
