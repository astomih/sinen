#include "luaapi.hpp"
#include <core/time/timer.hpp>

namespace sinen {
void registerTimer(luau::table &sn) {
  engineClass<Timer>(sn, Timer::metaTableName())
      .static_function("new",
                       [](std::optional<float> time) {
                         return time ? Timer(*time) : Timer();
                       })
      .method<&Timer::start>("start")
      .method<&Timer::stop>("stop")
      .method<&Timer::isStarted>("isStarted")
      .method<&Timer::setTime>("setTime")
      .method<&Timer::check>("check");
}
} // namespace sinen
