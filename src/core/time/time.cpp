#include <SDL3/SDL.h>
#include <core/time/time.hpp>
#include <script/luaapi.hpp>

namespace sinen {
float Time::seconds() { return static_cast<float>(SDL_GetTicks() / 1000.f); }

uint32_t Time::milli() { return SDL_GetTicks(); }
float Time::deltaTime() { return delta; }
void Time::update() {
  delta = (SDL_GetTicks() - prev) / 1000.0f;
  prev = SDL_GetTicks();
}

static int lTimeSeconds(lua_State *L) {
  lua_pushnumber(L, Time::seconds());
  return 1;
}
static int lTimeMilli(lua_State *L) {
  lua_pushinteger(L, static_cast<lua_Integer>(Time::milli()));
  return 1;
}
static int lTimeDelta(lua_State *L) {
  lua_pushnumber(L, Time::deltaTime());
  return 1;
}
void registerTime(lua_State *L) {
  pushSnNamed(L, "Time");
  luaPushcfunction2(L, lTimeSeconds);
  lua_setfield(L, -2, "seconds");
  luaPushcfunction2(L, lTimeMilli);
  lua_setfield(L, -2, "milli");
  luaPushcfunction2(L, lTimeDelta);
  lua_setfield(L, -2, "delta");
  lua_pop(L, 1);
}

} // namespace sinen
