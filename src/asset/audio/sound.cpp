// internal
#include "audio_system.hpp"
#include <asset/audio/sound.hpp>
#include <core/io/asset_io.hpp>

// external
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/vector_float3.hpp"
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

void Sound::load(std::string_view fileName) {
  data = std::make_unique<Data>();
  auto path = AssetIO::getFilePath(AssetType::Sound, fileName);
  ma_sound_init_from_file(AudioSystem::getEngine(), path.c_str(),
                          MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC, nullptr,
                          nullptr, &data->sound);
}
void Sound::loadFromPath(std::string_view path) {
  data = std::make_unique<Data>();
  ma_sound_init_from_file(AudioSystem::getEngine(), path.data(),
                          MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC, nullptr,
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

void Sound::setPosition(const glm::vec3 &pos) const {
  ma_sound_set_position(&data->sound, pos.x, pos.y, pos.z);
}
void Sound::setDirection(const glm::vec3 &dir) const {
  ma_sound_set_direction(&data->sound, dir.x, dir.y, dir.z);
}
bool Sound::isPlaying() const { return ma_sound_is_playing(&data->sound); }
bool Sound::isLooping() const { return ma_sound_is_looping(&data->sound); }

float Sound::getVolume() const { return ma_sound_get_volume(&data->sound); }

float Sound::getPitch() const { return ma_sound_get_pitch(&data->sound); }

glm::vec3 Sound::getPosition() const {
  auto temp = ma_sound_get_position(&data->sound);
  return {temp.x, temp.y, temp.z};
}
glm::vec3 Sound::getDirection() const {
  auto temp = ma_sound_get_direction(&data->sound);
  return {temp.x, temp.y, temp.z};
}
} // namespace sinen
