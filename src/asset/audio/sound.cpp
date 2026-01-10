// internal
#include "audio.hpp"
#include <asset/audio/sound.hpp>
#include <memory>
#include <platform/io/asset_io.hpp>

// external
#include "core/data/ptr.hpp"
#if 1
#define MINIAUDIO_IMPLEMENTATION
#endif
#include <miniaudio.h>

namespace sinen {
Sound::Sound() : sound() {}
Sound::~Sound() { ma_sound_uninit(&sound); }

void Sound::load(StringView fileName) {
  auto path = AssetIO::getFilePath(fileName);
  ma_sound_init_from_file(Audio::getEngine(), path.c_str(),
                          MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC, nullptr,
                          nullptr, &sound);
}
void Sound::load(const Buffer &buffer) {
  ma_decoder decoder;
  ma_decoder_config dcfg = ma_decoder_config_init(ma_format_f32, 2, 44100);
  ma_result r =
      ma_decoder_init_memory(buffer.data(), buffer.size(), &dcfg, &decoder);
  if (r != MA_SUCCESS) {
    return;
  }
  ma_sound_init_from_data_source(Audio::getEngine(), &decoder,
                                 MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC,
                                 nullptr, &sound);
}
void Sound::play() { ma_sound_start(&sound); }

void Sound::restart() {
  ma_data_source_seek_to_pcm_frame(sound.pDataSource, 0);
}

void Sound::stop() { ma_sound_stop(&sound); }
void Sound::setLooping(bool looping) { ma_sound_set_looping(&sound, looping); }

void Sound::setVolume(float value) { ma_sound_set_volume(&sound, value); }

void Sound::setPitch(float value) { ma_sound_set_pitch(&sound, value); }

void Sound::setPosition(const Vec3 &pos) {
  ma_sound_set_position(&sound, pos.x, pos.y, pos.z);
}
void Sound::setDirection(const Vec3 &dir) {
  ma_sound_set_direction(&sound, dir.x, dir.y, dir.z);
}
bool Sound::isPlaying() const { return ma_sound_is_playing(&sound); }
bool Sound::isLooping() const { return ma_sound_is_looping(&sound); }

float Sound::getVolume() const { return ma_sound_get_volume(&sound); }

float Sound::getPitch() const { return ma_sound_get_pitch(&sound); }

Vec3 Sound::getPosition() const {
  auto temp = ma_sound_get_position(&sound);
  return Vec3{temp.x, temp.y, temp.z};
}
Vec3 Sound::getDirection() const {
  auto temp = ma_sound_get_direction(&sound);
  return Vec3{temp.x, temp.y, temp.z};
}
} // namespace sinen
