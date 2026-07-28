#include "luaapi.hpp"
#include <audio/synth.hpp>
#include <core/logger/log.hpp>

namespace sinen {
static int lSynthNew(lua_State *L) {
  udPushPtr<Synth>(L, Synth::create());
  return 1;
}
static int lSynthPlay(lua_State *L) {
  udPtr<Synth>(L, 1)->play();
  return 0;
}
static int lSynthStop(lua_State *L) {
  udPtr<Synth>(L, 1)->stop();
  return 0;
}
static int lSynthSetPattern(lua_State *L) {
  auto &s = udPtr<Synth>(L, 1);
  const char *p = luaL_checkstring(L, 2);
  s->setPattern(StringView(p));
  return 0;
}
static int lSynthSetBpm(lua_State *L) {
  auto &s = udPtr<Synth>(L, 1);
  float v = static_cast<float>(luaL_checknumber(L, 2));
  s->setBpm(v);
  return 0;
}
static int lSynthSetStepBeats(lua_State *L) {
  auto &s = udPtr<Synth>(L, 1);
  float v = static_cast<float>(luaL_checknumber(L, 2));
  s->setStepBeats(v);
  return 0;
}
static int lSynthSetMasterGain(lua_State *L) {
  auto &s = udPtr<Synth>(L, 1);
  float v = static_cast<float>(luaL_checknumber(L, 2));
  s->setMasterGain(v);
  return 0;
}
static int lSynthSetADSR(lua_State *L) {
  auto &s = udPtr<Synth>(L, 1);
  float a = static_cast<float>(luaL_checknumber(L, 2));
  float d = static_cast<float>(luaL_checknumber(L, 3));
  float su = static_cast<float>(luaL_checknumber(L, 4));
  float r = static_cast<float>(luaL_checknumber(L, 5));
  s->setADSR(a, d, su, r);
  return 0;
}

void registerSynth(lua_State *L) {
  luaL_newmetatable(L, Synth::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  Binding::registerFunction(L, "play", lSynthPlay);
  Binding::registerFunction(L, "stop", lSynthStop);
  Binding::registerFunction(L, "setPattern", lSynthSetPattern);
  Binding::registerFunction(L, "setBpm", lSynthSetBpm);
  Binding::registerFunction(L, "setStepBeats", lSynthSetStepBeats);
  Binding::registerFunction(L, "setMasterGain", lSynthSetMasterGain);
  Binding::registerFunction(L, "setADSR", lSynthSetADSR);

  lua_pop(L, 1);

  pushSnNamed(L, "Synth");
  Binding::registerFunction(L, "new", lSynthNew);
  lua_pop(L, 1);
}
} // namespace sinen
