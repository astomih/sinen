#include "luaapi.hpp"
#include <core/time/time.hpp>

namespace sinen {
static auto lTimeSeconds() { return Time::seconds(); }
static auto lTimeMilli() { return static_cast<lua_Integer>(Time::milli()); }
static auto lTimeDelta() { return Time::deltaTime(); }
void registerTime(luau::table &sn) {
  auto time = sn.get_or_create_table("Time");
  time.function<lTimeSeconds>("seconds");
  time.function<lTimeMilli>("milli");
  time.function<lTimeDelta>("delta");
}
} // namespace sinen
