#include "luaapi.hpp"
#include <core/time/timer.hpp>

namespace sinen {
void registerTimer(lua_State *L) {
  engineClass<Timer>(L, Timer::metaTableName())
      .static_function("new",
                       [](std::optional<float> time) {
                         return time ? Timer(*time) : Timer();
                       })
      .method("start", &Timer::start)
      .method("stop", &Timer::stop)
      .method("isStarted", &Timer::isStarted)
      .method("setTime", &Timer::setTime)
      .method("check", &Timer::check);
}
} // namespace sinen
