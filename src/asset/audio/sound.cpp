// internal
#include "audio_system.hpp"
#include <asset/audio/sound.hpp>

// external
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include <miniaudio.h>

namespace sinen {
struct Sound::Data {
  ma_sound sound;
};
Sound::Sound() {}
Sound::~Sound() { ma_sound_uninit(&data->sound); }

void Sound::load(std::string_view fileName) {
  data = std::make_unique<Data>();
  AudioSystem::load(&data->sound, fileName);
}
void Sound::loadFromPath(std::string_view path) {}
void Sound::play() {
  ma_sound_stop(&data->sound);
  ma_sound_start(&data->sound);
}
void Sound::newSource() {}
void Sound::setListener(glm::vec3 pos, glm::vec3 rotation) {
  glm::quat q = glm::angleAxis(rotation.z, glm::vec3(0, 0, -1));
  q = q * glm::angleAxis(rotation.y, glm::vec3(0, 1, 0));
  q = q * glm::angleAxis(rotation.x, glm::vec3(1, 0, 0));

  AudioSystem::setListener(pos, q);
}
void Sound::deleteSource() {}
bool Sound::isValid() {
  return AudioSystem::get_buffers().contains(mName.data());
}

void Sound::restart() {}

void Sound::stop(bool allowFadeOut /* true */) {}

void Sound::setPaused(bool pause) {}

void Sound::setVolume(float value) {}

void Sound::setPitch(float value) {}

void Sound::setPosition(glm::vec3 pos) {}

bool Sound::getPaused() { return isPaused; }

float Sound::getVolume() { return volume; }

float Sound::getPitch() { return pitch; }

const glm::vec3 &Sound::getPosition() { return pos; }

std::string Sound::getName() { return mName; }

} // namespace sinen
