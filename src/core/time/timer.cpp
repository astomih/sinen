#include <core/time/time.hpp>
#include <core/time/timer.hpp>
#include <script/luaapi.hpp>
namespace sinen {
Timer::Timer() : startTime(0.f), currentTime(0.f), isStartedTimer(false) {}
Timer::Timer(float time) : startTime(0.f), currentTime(0.f), isStartedTimer(false) {
  setTime(time);
}
Timer::~Timer() = default;
void Timer::start() {
  startTime = Time::milli();
  isStartedTimer = true;
}
void Timer::setTime(float milliSecond) { this->currentTime = milliSecond; }
bool Timer::check() {
  return !isStartedTimer ? false : currentTime <= Time::milli() - startTime;
}
void Timer::stop() { isStartedTimer = false; }

static int lTimerNew(lua_State *L) {
  if (lua_gettop(L) >= 1 && lua_isnumber(L, 1)) {
    float t = static_cast<float>(lua_tonumber(L, 1));
    udNewOwned<Timer>(L, Timer(t));
    return 1;
  }
  udNewOwned<Timer>(L, Timer());
  return 1;
}
static int lTimerStart(lua_State *L) {
  udValue<Timer>(L, 1).start();
  return 0;
}
static int lTimerStop(lua_State *L) {
  udValue<Timer>(L, 1).stop();
  return 0;
}
static int lTimerIsStarted(lua_State *L) {
  lua_pushboolean(L, udValue<Timer>(L, 1).isStarted());
  return 1;
}
static int lTimerSetTime(lua_State *L) {
  float ms = static_cast<float>(luaL_checknumber(L, 2));
  udValue<Timer>(L, 1).setTime(ms);
  return 0;
}
static int lTimerCheck(lua_State *L) {
  lua_pushboolean(L, udValue<Timer>(L, 1).check());
  return 1;
}
void registerTimer(lua_State *L) {
  luaL_newmetatable(L, Timer::metaTableName());
  luaPushcfunction2(L, udGc<Timer>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lTimerStart);
  lua_setfield(L, -2, "start");
  luaPushcfunction2(L, lTimerStop);
  lua_setfield(L, -2, "stop");
  luaPushcfunction2(L, lTimerIsStarted);
  lua_setfield(L, -2, "isStarted");
  luaPushcfunction2(L, lTimerSetTime);
  lua_setfield(L, -2, "setTime");
  luaPushcfunction2(L, lTimerCheck);
  lua_setfield(L, -2, "check");
  lua_pop(L, 1);

  pushSnNamed(L, "Timer");
  luaPushcfunction2(L, lTimerNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
