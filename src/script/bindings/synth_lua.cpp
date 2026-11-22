#include "luaapi.hpp"
#include <audio/synth.hpp>
#include <core/logger/log.hpp>

namespace sinen {
static auto lSynthNew() { return Synth::create(); }
static int lSynthPlay(lua_State *L) {
  udPtr<Synth>(L, 1)->play();
  return 0;
}
static int lSynthStop(lua_State *L) {
  udPtr<Synth>(L, 1)->stop();
  return 0;
}
static auto lSynthSetPattern(Ptr<Synth> &s, const char *p) {
  s->setPattern(StringView(p));
  return;
}
static auto lSynthSetBpm(Ptr<Synth> &s, float v) {
  s->setBpm(v);
  return;
}
static auto lSynthSetStepBeats(Ptr<Synth> &s, float v) {
  s->setStepBeats(v);
  return;
}
static auto lSynthSetMasterGain(Ptr<Synth> &s, float v) {
  s->setMasterGain(v);
  return;
}
static auto lSynthSetADSR(Ptr<Synth> &s, float a, float d, float su, float r) {
  s->setADSR(a, d, su, r);
  return;
}

void registerSynth(lua_State *L) {
  pushClassMetatable<Ptr<Synth>>(L, Synth::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  luau::table(L, -1).raw_function("play", lSynthPlay);
  luau::table(L, -1).raw_function("stop", lSynthStop);
  luau::table(L, -1).function("setPattern", lSynthSetPattern);
  luau::table(L, -1).function("setBpm", lSynthSetBpm);
  luau::table(L, -1).function("setStepBeats", lSynthSetStepBeats);
  luau::table(L, -1).function("setMasterGain", lSynthSetMasterGain);
  luau::table(L, -1).function("setADSR", lSynthSetADSR);

  lua_pop(L, 1);

  pushSnNamed(L, "Synth");
  luau::table(L, -1).function("new", lSynthNew);
  lua_pop(L, 1);
}
} // namespace sinen
