// internal
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include "sound_system.hpp"
#include <asset/audio/music.hpp>
#include <asset/audio/sound.hpp>

// external
#define AL_LIBTYPE_STATIC
#include <AL/al.h>
#include <AL/alc.h>

namespace sinen {
Sound::Sound() {}
void Sound::load(std::string_view file_name) {
  SoundSystem::load(file_name);
  auto sourceID = SoundSystem::new_source(file_name);
  ALint buf;
  alGetSourcei(sourceID, AL_BUFFER, &buf);
  param.source_id = sourceID;
  param.buffer_id = buf;
  mName = file_name.data();
}
void Sound::loadFromPath(std::string_view path) {
  SoundSystem::load(path);
  auto sourceID = SoundSystem::new_source(path);
  ALint buf;
  alGetSourcei(sourceID, AL_BUFFER, &buf);
  param.source_id = sourceID;
  param.buffer_id = buf;
  mName = path.data();
}
void Sound::play() { alSourcePlay(param.source_id); }
void Sound::newSource() {
  auto sourceID = SoundSystem::new_source(mName);
  ALint buf;
  alGetSourcei(sourceID, AL_BUFFER, &buf);
  param.source_id = sourceID;
  param.buffer_id = buf;
}
void Sound::setListener(glm::vec3 pos, glm::vec3 rotation) {
  glm::quat q = glm::angleAxis(rotation.z, glm::vec3(0, 0, -1));
  q = q * glm::angleAxis(rotation.y, glm::vec3(0, 1, 0));
  q = q * glm::angleAxis(rotation.x, glm::vec3(1, 0, 0));

  SoundSystem::set_listener(pos, q);
}
void Sound::deleteSource() { SoundSystem::delete_source(param.source_id); }
bool Sound::isValid() {
  return SoundSystem::get_buffers().contains(mName.data());
}

void Sound::restart() {
  isPlaying = true;
  stop();
  alSourcePlay(param.source_id);
}

void Sound::stop(bool allowFadeOut /* true */) {
  isPlaying = false;
  alSourceStop(param.source_id);
}

void Sound::setPaused(bool pause) {
  isPaused = pause;
  alSourcePause(param.source_id);
}

void Sound::setVolume(float value) {
  volume = value;
  alSourcef(param.source_id, AL_GAIN, value);
}

void Sound::setPitch(float value) {
  pitch = value;
  alSourcef(param.source_id, AL_PITCH, value);
}

void Sound::setPosition(glm::vec3 pos) {
  this->pos = pos;
  alSource3f(param.source_id, AL_POSITION, pos.x, pos.y, pos.z);
}

bool Sound::getPaused() { return isPaused; }

float Sound::getVolume() { return volume; }

float Sound::getPitch() { return pitch; }

const glm::vec3 &Sound::getPosition() { return pos; }

std::string Sound::getName() { return mName; }

} // namespace sinen
