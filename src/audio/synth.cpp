// internal
#include "audio.hpp"
#include <audio/synth.hpp>
#include <core/logger/log.hpp>

// external
#include <miniaudio.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string_view>
#include <utility>
#include <vector>

namespace sinen {
namespace {
constexpr float kTwoPi = 6.2831853071795864769f;

enum class Wave : uint8_t { Sine, Saw, Square, Tri, Noise };

static Wave parseWave(std::string_view s) {
  if (s == "sine")
    return Wave::Sine;
  if (s == "saw")
    return Wave::Saw;
  if (s == "square" || s == "sq")
    return Wave::Square;
  if (s == "tri" || s == "triangle")
    return Wave::Tri;
  if (s == "noise")
    return Wave::Noise;
  return Wave::Sine;
}

static float midiToHz(int midi) {
  return 440.0f * std::pow(2.0f, (static_cast<float>(midi) - 69.0f) / 12.0f);
}

static bool isWs(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',';
}

static int noteToMidi(std::string_view tok) {
  auto isDigit = [](char c) { return c >= '0' && c <= '9'; };
  if (!tok.empty() && (isDigit(tok[0]) || tok[0] == '-')) {
    int midi = std::stoi(std::string(tok));
    return std::clamp(midi, 0, 127);
  }

  if (tok.empty())
    return 60;

  char n0 = static_cast<char>(std::tolower(static_cast<unsigned char>(tok[0])));
  int base = 0;
  switch (n0) {
  case 'c':
    base = 0;
    break;
  case 'd':
    base = 2;
    break;
  case 'e':
    base = 4;
    break;
  case 'f':
    base = 5;
    break;
  case 'g':
    base = 7;
    break;
  case 'a':
    base = 9;
    break;
  case 'b':
    base = 11;
    break;
  default:
    return 60;
  }

  size_t i = 1;
  if (i < tok.size()) {
    if (tok[i] == '#') {
      base += 1;
      i++;
    } else if (tok[i] == 'b') {
      base -= 1;
      i++;
    }
  }
  while (base < 0)
    base += 12;
  base %= 12;

  int octave = 4;
  if (i < tok.size()) {
    octave = std::stoi(std::string(tok.substr(i)));
  }

  // C4 = 60
  int midi = (octave + 1) * 12 + base;
  return std::clamp(midi, 0, 127);
}

struct PatternEvent {
  float beat = 0.0f;
  float durBeats = 0.25f;
  int midi = 60;
  float vel = 0.8f;
  Wave wave = Wave::Sine;
  float cutoffHz = 2000.0f;
  float pan = 0.0f; // -1..1
};

struct PatternData {
  std::vector<PatternEvent> events;
  float lengthBeats = 4.0f;
};

struct Node {
  enum class Type : uint8_t { Rest, Note, List } type = Type::Rest;
  // Note fields
  int midi = 60;
  float vel = 0.8f;
  float durOverrideBeats = -1.0f;
  Wave wave = Wave::Sine;
  float cutoffHz = 2000.0f;
  float pan = 0.0f;
  std::vector<Node> children;
};

struct PatternParser {
  std::string_view s;
  size_t pos = 0;

  void skipWs() {
    while (pos < s.size() && isWs(s[pos]))
      pos++;
  }

  bool eof() const { return pos >= s.size(); }

  Node parseElement() {
    skipWs();
    if (eof())
      return Node{};

    if (s[pos] == '[') {
      pos++; // '['
      Node n;
      n.type = Node::Type::List;
      for (;;) {
        skipWs();
        if (eof())
          break;
        if (s[pos] == ']') {
          pos++;
          break;
        }
        n.children.push_back(parseElement());
      }
      return n;
    }

    // Atom token.
    size_t start = pos;
    while (pos < s.size() && !isWs(s[pos]) && s[pos] != ']' && s[pos] != '[') {
      pos++;
    }
    auto tok = s.substr(start, pos - start);

    Node n;
    if (tok == "~" || tok == "." || tok.empty()) {
      n.type = Node::Type::Rest;
      return n;
    }
    n.type = Node::Type::Note;

    // token modifiers: <note>[:dur][@vel][^wave][#cutoff][!pan]
    auto cut = [&](char c) -> std::string_view {
      auto idx = tok.find(c);
      if (idx == std::string_view::npos)
        return {};
      auto end = tok.size();
      for (char stopper : {':', '@', '^', '#', '!'}) {
        if (stopper == c)
          continue;
        auto j = tok.find(stopper, idx + 1);
        if (j != std::string_view::npos)
          end = std::min(end, j);
      }
      return tok.substr(idx + 1, end - (idx + 1));
    };

    auto baseEnd = tok.size();
    for (char stopper : {':', '@', '^', '#', '!'}) {
      auto j = tok.find(stopper);
      if (j != std::string_view::npos)
        baseEnd = std::min(baseEnd, j);
    }
    auto baseTok = tok.substr(0, baseEnd);
    n.midi = noteToMidi(baseTok);

    if (auto dur = cut(':'); !dur.empty()) {
      n.durOverrideBeats = std::stof(std::string(dur));
    }
    if (auto vel = cut('@'); !vel.empty()) {
      n.vel = std::clamp(std::stof(std::string(vel)), 0.0f, 1.0f);
    }
    if (auto w = cut('^'); !w.empty()) {
      n.wave = parseWave(w);
    }
    if (auto c = cut('#'); !c.empty()) {
      n.cutoffHz = std::max(20.0f, std::stof(std::string(c)));
    }
    if (auto p = cut('!'); !p.empty()) {
      n.pan = std::clamp(std::stof(std::string(p)), -1.0f, 1.0f);
    }

    return n;
  }
};

static void flatten(const Node &n, float startBeat, float durBeats,
                    std::vector<PatternEvent> &out) {
  if (durBeats <= 0.0f)
    return;

  if (n.type == Node::Type::Rest) {
    return;
  }
  if (n.type == Node::Type::Note) {
    PatternEvent e;
    e.beat = startBeat;
    e.durBeats = (n.durOverrideBeats > 0.0f) ? n.durOverrideBeats : durBeats;
    e.midi = n.midi;
    e.vel = n.vel;
    e.wave = n.wave;
    e.cutoffHz = n.cutoffHz;
    e.pan = n.pan;
    out.push_back(e);
    return;
  }

  if (n.children.empty())
    return;
  float subDur = durBeats / static_cast<float>(n.children.size());
  for (size_t i = 0; i < n.children.size(); i++) {
    flatten(n.children[i], startBeat + static_cast<float>(i) * subDur, subDur,
            out);
  }
}

static PatternData compilePattern(StringView pattern, float stepBeats) {
  PatternData out;
  out.events.clear();
  out.lengthBeats = 4.0f;

  PatternParser p{pattern, 0};
  std::vector<Node> top;
  while (!p.eof()) {
    p.skipWs();
    if (p.eof())
      break;
    top.push_back(p.parseElement());
  }

  if (top.empty()) {
    out.lengthBeats = 4.0f;
    return out;
  }

  out.lengthBeats =
      std::max(stepBeats * static_cast<float>(top.size()), 0.001f);
  float t = 0.0f;
  for (auto &n : top) {
    flatten(n, t, stepBeats, out.events);
    t += stepBeats;
  }

  std::sort(out.events.begin(), out.events.end(),
            [](const PatternEvent &a, const PatternEvent &b) {
              return a.beat < b.beat;
            });
  return out;
}

enum class EnvStage : uint8_t { Off, Attack, Decay, Sustain, Release };

struct Voice {
  bool active = false;
  Wave wave = Wave::Sine;
  float freqHz = 440.0f;
  float phase = 0.0f;
  float vel = 0.8f;
  float pan = 0.0f;
  float cutoffHz = 2000.0f;

  uint64_t startFrame = 0;
  uint64_t noteOffFrame = 0;

  EnvStage envStage = EnvStage::Off;
  float env = 0.0f;

  float lp = 0.0f;

  uint32_t noise = 0x12345678u;
};

struct SynthDataSource {
  ma_data_source_base base;
  class SynthImpl *owner = nullptr;
};

class SynthImpl : public Synth {
public:
  SynthImpl() {
    auto *eng = static_cast<ma_engine *>(Audio::getEngine());
    sampleRate = ma_engine_get_sample_rate(eng);
    channels = ma_engine_get_channels(eng);

    initDataSource();

    // Create a sound from the data source. It will be started via play().
    if (ma_sound_init_from_data_source(eng, &ds.base, 0, nullptr, &sound) !=
        MA_SUCCESS) {
      Log::error("Synth: ma_sound_init_from_data_source failed");
    }
    ma_sound_set_spatialization_enabled(&sound, MA_FALSE);
  }

  ~SynthImpl() override {
    ma_sound_uninit(&sound);
    ma_data_source_uninit(&ds.base);
  }

  void play() override {
    resetPlayback();
    ma_sound_start(&sound);
  }

  void stop() override { ma_sound_stop(&sound); }

  void setPattern(StringView pattern) override {
    float sb = std::max(stepBeats.load(std::memory_order_relaxed), 0.001f);
    int inactive = 1 - activePatternIndex.load(std::memory_order_relaxed);
    patterns[inactive] = compilePattern(pattern, sb);
    activePatternIndex.store(inactive, std::memory_order_release);
    patternChangeCounter.fetch_add(1, std::memory_order_release);
  }

  void setBpm(float bpm_) override {
    bpm.store(std::clamp(bpm_, 1.0f, 999.0f), std::memory_order_relaxed);
  }

  void setStepBeats(float beatsPerStep) override {
    stepBeats.store(std::clamp(beatsPerStep, 1.0f / 64.0f, 16.0f),
                    std::memory_order_relaxed);
  }

  void setMasterGain(float gain) override {
    masterGain.store(std::clamp(gain, 0.0f, 4.0f), std::memory_order_relaxed);
  }

  void setADSR(float a, float d, float s, float r) override {
    attack.store(std::clamp(a, 0.0f, 10.0f), std::memory_order_relaxed);
    decay.store(std::clamp(d, 0.0f, 10.0f), std::memory_order_relaxed);
    sustain.store(std::clamp(s, 0.0f, 1.0f), std::memory_order_relaxed);
    release.store(std::clamp(r, 0.0f, 10.0f), std::memory_order_relaxed);
  }

  ma_result onRead(void *pFramesOut, ma_uint64 frameCount,
                   ma_uint64 *pFramesRead) {
    if (pFramesRead)
      *pFramesRead = frameCount;

    auto *out = static_cast<float *>(pFramesOut);
    const uint32_t ch = channels;
    if (out == nullptr) {
      advanceNoOutput(frameCount);
      return MA_SUCCESS;
    }

    // Handle pattern swaps.
    uint32_t pc = patternChangeCounter.load(std::memory_order_acquire);
    if (pc != lastSeenPatternChange) {
      lastSeenPatternChange = pc;
      resetPlaybackStateKeepVoices();
    }

    const PatternData &pat =
        patterns[activePatternIndex.load(std::memory_order_acquire)];
    const float patLen = std::max(pat.lengthBeats, 0.001f);

    float bpmNow = bpm.load(std::memory_order_relaxed);
    float secondsPerBeat = 60.0f / bpmNow;
    float beatInc = 1.0f / (secondsPerBeat * static_cast<float>(sampleRate));

    float gain = masterGain.load(std::memory_order_relaxed);

    float a = attack.load(std::memory_order_relaxed);
    float d = decay.load(std::memory_order_relaxed);
    float sus = sustain.load(std::memory_order_relaxed);
    float rel = release.load(std::memory_order_relaxed);

    float aInc =
        (a <= 0.0f) ? 1.0f : 1.0f / (a * static_cast<float>(sampleRate));
    float dInc =
        (d <= 0.0f) ? 1.0f : 1.0f / (d * static_cast<float>(sampleRate));
    float rInc =
        (rel <= 0.0f) ? 1.0f : 1.0f / (rel * static_cast<float>(sampleRate));

    for (ma_uint64 f = 0; f < frameCount; f++) {
      // Trigger pattern events.
      while (nextEventIndex < pat.events.size() &&
             beatPos >= pat.events[nextEventIndex].beat) {
        triggerVoice(pat.events[nextEventIndex], cursorFrame);
        nextEventIndex++;
      }

      // Mix voices.
      float l = 0.0f;
      float r = 0.0f;
      for (auto &v : voices) {
        if (!v.active)
          continue;

        if (cursorFrame >= v.noteOffFrame && v.envStage != EnvStage::Release &&
            v.envStage != EnvStage::Off) {
          v.envStage = EnvStage::Release;
        }

        // Envelope.
        switch (v.envStage) {
        case EnvStage::Off:
          v.active = false;
          continue;
        case EnvStage::Attack:
          v.env += aInc;
          if (v.env >= 1.0f) {
            v.env = 1.0f;
            v.envStage = EnvStage::Decay;
          }
          break;
        case EnvStage::Decay:
          v.env -= dInc;
          if (v.env <= sus) {
            v.env = sus;
            v.envStage = EnvStage::Sustain;
          }
          break;
        case EnvStage::Sustain:
          v.env = sus;
          break;
        case EnvStage::Release:
          v.env -= rInc;
          if (v.env <= 0.0f) {
            v.env = 0.0f;
            v.envStage = EnvStage::Off;
            v.active = false;
            continue;
          }
          break;
        }

        // Oscillator.
        float x = 0.0f;
        switch (v.wave) {
        case Wave::Sine:
          x = std::sinf(kTwoPi * v.phase);
          break;
        case Wave::Saw:
          x = 2.0f * (v.phase - std::floor(v.phase + 0.5f));
          break;
        case Wave::Square:
          x = (v.phase < 0.5f) ? 1.0f : -1.0f;
          break;
        case Wave::Tri: {
          float saw = 2.0f * (v.phase - std::floor(v.phase + 0.5f));
          x = 2.0f * std::fabs(saw) - 1.0f;
        } break;
        case Wave::Noise:
          v.noise ^= v.noise << 13;
          v.noise ^= v.noise >> 17;
          v.noise ^= v.noise << 5;
          x = (static_cast<int32_t>(v.noise) / 2147483648.0f);
          break;
        }

        v.phase += v.freqHz / static_cast<float>(sampleRate);
        if (v.phase >= 1.0f)
          v.phase -= std::floor(v.phase);

        // Simple 1-pole lowpass.
        float cutoff = std::clamp(v.cutoffHz, 20.0f,
                                  0.45f * static_cast<float>(sampleRate));
        float alpha =
            1.0f - std::exp(-kTwoPi * cutoff / static_cast<float>(sampleRate));
        v.lp += alpha * (x - v.lp);

        float s0 = v.lp * v.env * v.vel;

        float pan = std::clamp(v.pan, -1.0f, 1.0f);
        float gl = 0.5f * (1.0f - pan);
        float gr = 0.5f * (1.0f + pan);

        l += s0 * gl;
        r += s0 * gr;
      }

      // Soft clip.
      l = l / (1.0f + std::fabs(l));
      r = r / (1.0f + std::fabs(r));

      l *= gain;
      r *= gain;

      if (ch == 1) {
        out[0] = 0.5f * (l + r);
        out += 1;
      } else {
        out[0] = l;
        out[1] = r;
        out += 2;
      }

      cursorFrame++;
      beatPos += beatInc;

      if (beatPos >= patLen) {
        beatPos -= patLen * std::floor(beatPos / patLen);
        nextEventIndex = 0;
      }
    }

    return MA_SUCCESS;
  }

  ma_result onSeek(ma_uint64 frameIndex) {
    (void)frameIndex;
    resetPlayback();
    return MA_SUCCESS;
  }

  ma_result onGetDataFormat(ma_format *pFormat, ma_uint32 *pChannels,
                            ma_uint32 *pSampleRate, ma_channel *pChannelMap,
                            size_t channelMapCap) {
    if (pFormat)
      *pFormat = ma_format_f32;
    if (pChannels)
      *pChannels = channels;
    if (pSampleRate)
      *pSampleRate = sampleRate;
    if (pChannelMap && channelMapCap >= channels) {
      if (channels == 1) {
        pChannelMap[0] = MA_CHANNEL_MONO;
      } else if (channels == 2) {
        pChannelMap[0] = MA_CHANNEL_FRONT_LEFT;
        pChannelMap[1] = MA_CHANNEL_FRONT_RIGHT;
      }
    }
    return MA_SUCCESS;
  }

  ma_result onGetCursor(ma_uint64 *pCursor) {
    if (!pCursor)
      return MA_INVALID_ARGS;
    *pCursor = cursorFrame;
    return MA_SUCCESS;
  }

  ma_result onGetLength(ma_uint64 *pLength) {
    if (!pLength)
      return MA_INVALID_ARGS;
    *pLength = 0;
    return MA_NOT_IMPLEMENTED;
  }

private:
  void initDataSource() {
    static ma_data_source_vtable vtable = {
        [](ma_data_source *pDataSource, void *pFramesOut, ma_uint64 frameCount,
           ma_uint64 *pFramesRead) -> ma_result {
          auto *self = reinterpret_cast<SynthDataSource *>(pDataSource)->owner;
          return self->onRead(pFramesOut, frameCount, pFramesRead);
        },
        [](ma_data_source *pDataSource, ma_uint64 frameIndex) -> ma_result {
          auto *self = reinterpret_cast<SynthDataSource *>(pDataSource)->owner;
          return self->onSeek(frameIndex);
        },
        [](ma_data_source *pDataSource, ma_format *pFormat,
           ma_uint32 *pChannels, ma_uint32 *pSampleRate,
           ma_channel *pChannelMap, size_t channelMapCap) -> ma_result {
          auto *self = reinterpret_cast<SynthDataSource *>(pDataSource)->owner;
          return self->onGetDataFormat(pFormat, pChannels, pSampleRate,
                                       pChannelMap, channelMapCap);
        },
        [](ma_data_source *pDataSource, ma_uint64 *pCursor) -> ma_result {
          auto *self = reinterpret_cast<SynthDataSource *>(pDataSource)->owner;
          return self->onGetCursor(pCursor);
        },
        [](ma_data_source *pDataSource, ma_uint64 *pLength) -> ma_result {
          auto *self = reinterpret_cast<SynthDataSource *>(pDataSource)->owner;
          return self->onGetLength(pLength);
        },
        nullptr, // onSetLooping
        0,
    };

    ma_data_source_config cfg = ma_data_source_config_init();
    cfg.vtable = &vtable;
    ds.owner = this;
    if (ma_data_source_init(&cfg, &ds.base) != MA_SUCCESS) {
      Log::error("Synth: ma_data_source_init failed");
    }
  }

  void resetPlayback() {
    cursorFrame = 0;
    beatPos = 0.0f;
    nextEventIndex = 0;
    for (auto &v : voices) {
      v.active = false;
      v.envStage = EnvStage::Off;
      v.env = 0.0f;
      v.phase = 0.0f;
      v.lp = 0.0f;
    }
  }

  void resetPlaybackStateKeepVoices() {
    beatPos = 0.0f;
    nextEventIndex = 0;
  }

  void advanceNoOutput(ma_uint64 frameCount) {
    cursorFrame += frameCount;
    beatPos += static_cast<float>(frameCount) *
               (bpm.load(std::memory_order_relaxed) / 60.0f) /
               static_cast<float>(sampleRate);

    const PatternData &pat =
        patterns[activePatternIndex.load(std::memory_order_acquire)];
    const float patLen = std::max(pat.lengthBeats, 0.001f);
    if (beatPos >= patLen) {
      beatPos -= patLen * std::floor(beatPos / patLen);
      nextEventIndex = 0;
    }
  }

  void triggerVoice(const PatternEvent &e, uint64_t frameNow) {
    uint64_t durFrames =
        static_cast<uint64_t>(std::max(0.0f, e.durBeats) *
                              (60.0f / bpm.load(std::memory_order_relaxed)) *
                              static_cast<float>(sampleRate));

    Voice *v = nullptr;
    for (auto &vv : voices) {
      if (!vv.active) {
        v = &vv;
        break;
      }
    }
    if (!v) {
      // Steal oldest.
      v = &voices[0];
      for (auto &vv : voices) {
        if (vv.startFrame < v->startFrame)
          v = &vv;
      }
    }

    v->active = true;
    v->wave = e.wave;
    v->freqHz = midiToHz(e.midi);
    v->vel = std::clamp(e.vel, 0.0f, 1.0f);
    v->pan = e.pan;
    v->cutoffHz = e.cutoffHz;
    v->startFrame = frameNow;
    v->noteOffFrame = frameNow + std::max<uint64_t>(durFrames, 1);
    v->envStage = EnvStage::Attack;
    v->env = 0.0f;
    v->lp = 0.0f;
  }

private:
  SynthDataSource ds{};
  ma_sound sound{};

  ma_uint32 sampleRate = 48000;
  ma_uint32 channels = 2;

  std::atomic<float> bpm{120.0f};
  std::atomic<float> stepBeats{0.25f}; // 16th notes by default
  std::atomic<float> masterGain{0.6f};

  std::atomic<float> attack{0.005f};
  std::atomic<float> decay{0.08f};
  std::atomic<float> sustain{0.4f};
  std::atomic<float> release{0.12f};

  PatternData patterns[2];
  std::atomic<int> activePatternIndex{0};
  std::atomic<uint32_t> patternChangeCounter{1};

  // Audio thread state.
  uint32_t lastSeenPatternChange = 0;
  uint64_t cursorFrame = 0;
  float beatPos = 0.0f;
  size_t nextEventIndex = 0;

  static constexpr size_t kMaxVoices = 32;
  Voice voices[kMaxVoices]{};
};
} // namespace

Ptr<Synth> Synth::create() { return makePtr<SynthImpl>(); }
} // namespace sinen

#include <script/luaapi.hpp>
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
  luaPushcfunction2(L, udPtrGc<Synth>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  luaPushcfunction2(L, lSynthPlay);
  lua_setfield(L, -2, "play");
  luaPushcfunction2(L, lSynthStop);
  lua_setfield(L, -2, "stop");
  luaPushcfunction2(L, lSynthSetPattern);
  lua_setfield(L, -2, "setPattern");
  luaPushcfunction2(L, lSynthSetBpm);
  lua_setfield(L, -2, "setBpm");
  luaPushcfunction2(L, lSynthSetStepBeats);
  lua_setfield(L, -2, "setStepBeats");
  luaPushcfunction2(L, lSynthSetMasterGain);
  lua_setfield(L, -2, "setMasterGain");
  luaPushcfunction2(L, lSynthSetADSR);
  lua_setfield(L, -2, "setADSR");

  lua_pop(L, 1);

  pushSnNamed(L, "Synth");
  luaPushcfunction2(L, lSynthNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
