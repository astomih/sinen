#include "luaapi.hpp"

#include <LuaBridge/LuaBridge.h>

#include <math/random.hpp>
#include <math/vector.hpp>

namespace sinen {
void registerRandom(lua_State *L) {
  pushSnNamed(L, "Random");
  luabridge::getNamespaceFromStack(L)
      .addFunction("getRange", &Random::getRange)
      .addFunction("getIntRange", &Random::getIntRange);
  lua_pop(L, 1);
}

} // namespace sinen
