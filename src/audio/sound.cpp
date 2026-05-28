// internal
#include "audio.hpp"
#include <audio/sound.hpp>
#include <platform/io/asset_io.hpp>

// external
#include "core/data/array.hpp"
#include "core/data/ptr.hpp"
#if 1
#define MINIAUDIO_IMPLEMENTATION
#endif
#include <miniaudio.h>

#include <cstring>

namespace sinen {
class SoundImpl : public Sound {
public:
  SoundImpl() : sound() {}
  virtual ~SoundImpl() override { reset(); }

  void load(StringView fileName) override {
    if (AssetIO::isArchiveMounted() && AssetIO::exists(fileName)) {
      auto bytes = AssetIO::openAsString(fileName);
      loadMemory(bytes.data(), bytes.size());
      return;
    }

    reset();
    auto path = AssetIO::getFilePath(fileName);
    if (ma_sound_init_from_file((ma_engine *)Audio::getEngine(), path.c_str(),
                                MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC,
                                nullptr, nullptr, &sound) == MA_SUCCESS) {
      soundInitialized = true;
    }
  }
  void load(const Buffer &buffer) override {
    loadMemory(buffer.data(), static_cast<size_t>(buffer.size()));
  }
  void play() override { ma_sound_start(&sound); }

  void restart() override {
    ma_data_source_seek_to_pcm_frame(sound.pDataSource, 0);
  }

  void stop() override { ma_sound_stop(&sound); }
  void setLooping(bool looping) override {
    ma_sound_set_looping(&sound, looping);
  }

  void setVolume(float value) override { ma_sound_set_volume(&sound, value); }

  void setPitch(float value) override { ma_sound_set_pitch(&sound, value); }

  void setPosition(const Vec3 &pos) override {
    ma_sound_set_position(&sound, pos.x, pos.y, pos.z);
  }
  void setDirection(const Vec3 &dir) override {
    ma_sound_set_direction(&sound, dir.x, dir.y, dir.z);
  }
  bool isPlaying() const override { return ma_sound_is_playing(&sound); }
  bool isLooping() const override { return ma_sound_is_looping(&sound); }

  float getVolume() const override { return ma_sound_get_volume(&sound); }

  float getPitch() const override { return ma_sound_get_pitch(&sound); }

  Vec3 getPosition() const override {
    auto temp = ma_sound_get_position(&sound);
    return Vec3{temp.x, temp.y, temp.z};
  }
  Vec3 getDirection() const override {
    auto temp = ma_sound_get_direction(&sound);
    return Vec3{temp.x, temp.y, temp.z};
  }

private:
  void reset() {
    if (soundInitialized) {
      ma_sound_uninit(&sound);
      soundInitialized = false;
    }
    if (decoderInitialized) {
      ma_decoder_uninit(&decoder);
      decoderInitialized = false;
    }
    memory.clear();
  }

  void loadMemory(const void *data, size_t size) {
    reset();
    if (!data || size == 0) {
      return;
    }
    memory.resize(size);
    std::memcpy(memory.data(), data, size);

    ma_decoder_config dcfg = ma_decoder_config_init(ma_format_f32, 2, 44100);
    ma_result r =
        ma_decoder_init_memory(memory.data(), memory.size(), &dcfg, &decoder);
    if (r != MA_SUCCESS) {
      return;
    }
    decoderInitialized = true;
    r = ma_sound_init_from_data_source((ma_engine *)Audio::getEngine(),
                                       &decoder, 0, nullptr, &sound);
    if (r == MA_SUCCESS) {
      soundInitialized = true;
    }
  }

  ma_sound sound;
  ma_decoder decoder{};
  Array<char> memory;
  bool soundInitialized = false;
  bool decoderInitialized = false;
};
Ptr<Sound> Sound::create() { return makePtr<SoundImpl>(); }
} // namespace sinen

namespace sinen {
} // namespace sinen
