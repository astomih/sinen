#ifndef SINEN_SYNTH_HPP
#define SINEN_SYNTH_HPP
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>

namespace sinen {
class Synth {
public:
  static constexpr const char *metaTableName() { return "sn.Synth"; }
  static Ptr<Synth> create();

  virtual ~Synth() = default;

  virtual void play() = 0;
  virtual void stop() = 0;

  virtual void setPattern(StringView pattern) = 0;
  virtual void setBpm(float bpm) = 0;
  virtual void setStepBeats(float beatsPerStep) = 0;
  virtual void setMasterGain(float gain) = 0;
  virtual void setADSR(float attackSeconds, float decaySeconds, float sustain,
                       float releaseSeconds) = 0;
};
} // namespace sinen

#endif // SINEN_SYNTH_HPP
