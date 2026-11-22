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

void registerSynth(luau::table &sn) {
  auto synth = engineClass<Ptr<Synth>>(sn, Synth::metaTableName());

  synth.raw_method<lSynthPlay>("play");
  synth.raw_method<lSynthStop>("stop");
  synth.method<lSynthSetPattern>("setPattern");
  synth.method<lSynthSetBpm>("setBpm");
  synth.method<lSynthSetStepBeats>("setStepBeats");
  synth.method<lSynthSetMasterGain>("setMasterGain");
  synth.method<lSynthSetADSR>("setADSR");

  synth.static_function<lSynthNew>("new");
}
} // namespace sinen
