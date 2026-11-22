#include "luaapi.hpp"
#include <math/random.hpp>

namespace sinen {
void registerRandom(luau::table &sn) {
  auto random = sn.get_or_create_table("Random");
  random
      .function("getRange",
                [](float min, float max) { return Random::getRange(min, max); })
      .function("getIntRange",
                [](int min, int max) { return Random::getIntRange(min, max); });
}
} // namespace sinen
