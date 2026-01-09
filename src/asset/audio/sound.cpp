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
struct Sound::Data {
  ma_sound sound;
};
Sound::Sound() {}
Sound::~Sound() { ma_sound_uninit(&data->sound); }

void Sound::load(StringView fileName) {

  data = makeUnique<Data>();
  auto path = AssetIO::getFilePath(fileName);
  ma_sound_init_from_file(Audio::getEngine(), path.c_str(),
                          MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC, nullptr,
                          nullptr, &data->sound);
}
void Sound::load(const Buffer &buffer) {
  data = makeUnique<Data>();
  ma_decoder decoder;
  ma_decoder_config dcfg = ma_decoder_config_init(ma_format_f32, 2, 44100);
  ma_result r =
      ma_decoder_init_memory(buffer.data(), buffer.size(), &dcfg, &decoder);
  if (r != MA_SUCCESS) {
    return;
  }
  ma_sound_init_from_data_source(Audio::getEngine(), &decoder,
                                 MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC,
                                 nullptr, &data->sound);
}
void Sound::play() const { ma_sound_start(&data->sound); }

void Sound::restart() const {
  ma_data_source_seek_to_pcm_frame(data->sound.pDataSource, 0);
}

void Sound::stop() const { ma_sound_stop(&data->sound); }
void Sound::setLooping(bool looping) const {
  ma_sound_set_looping(&data->sound, looping);
}

void Sound::setVolume(float value) const {
  ma_sound_set_volume(&data->sound, value);
}

void Sound::setPitch(float value) const {
  ma_sound_set_pitch(&data->sound, value);
}

void Sound::setPosition(const Vec3 &pos) const {
  ma_sound_set_position(&data->sound, pos.x, pos.y, pos.z);
}
void Sound::setDirection(const Vec3 &dir) const {
  ma_sound_set_direction(&data->sound, dir.x, dir.y, dir.z);
}
bool Sound::isPlaying() const { return ma_sound_is_playing(&data->sound); }
bool Sound::isLooping() const { return ma_sound_is_looping(&data->sound); }

float Sound::getVolume() const { return ma_sound_get_volume(&data->sound); }

float Sound::getPitch() const { return ma_sound_get_pitch(&data->sound); }

Vec3 Sound::getPosition() const {
  auto temp = ma_sound_get_position(&data->sound);
  return Vec3{temp.x, temp.y, temp.z};
}
Vec3 Sound::getDirection() const {
  auto temp = ma_sound_get_direction(&data->sound);
  return Vec3{temp.x, temp.y, temp.z};
}
} // namespace sinen
