#include "luaapi.hpp"
#include <core/time/time.hpp>

namespace sinen {
static auto lTimeSeconds() { return Time::seconds(); }
static auto lTimeMilli() { return static_cast<lua_Integer>(Time::milli()); }
static auto lTimeDelta() { return Time::deltaTime(); }
void registerTime(lua_State *L) {
  pushSnNamed(L, "Time");
  luau::table(L, -1).function("seconds", lTimeSeconds);
  luau::table(L, -1).function("milli", lTimeMilli);
  luau::table(L, -1).function("delta", lTimeDelta);
  lua_pop(L, 1);
}

} // namespace sinen
