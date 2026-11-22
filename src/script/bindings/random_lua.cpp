#include "luaapi.hpp"
#include <math/random.hpp>

namespace sinen {
void registerRandom(lua_State *L) {
  pushSnNamed(L, "Random");
  luau::table(L, -1)
      .function("getRange",
                [](float min, float max) { return Random::getRange(min, max); })
      .function("getIntRange",
                [](int min, int max) { return Random::getIntRange(min, max); });
  lua_pop(L, 1);
}
} // namespace sinen
