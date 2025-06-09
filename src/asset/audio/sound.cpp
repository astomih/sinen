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
void Sound::Load(std::string_view file_name) {
  SoundSystem::load(file_name);
  auto sourceID = SoundSystem::new_source(file_name);
  ALint buf;
  alGetSourcei(sourceID, AL_BUFFER, &buf);
  param.source_id = sourceID;
  param.buffer_id = buf;
  mName = file_name.data();
}
void Sound::Play() { alSourcePlay(param.source_id); }
void Sound::NewSource() {
  auto sourceID = SoundSystem::new_source(mName);
  ALint buf;
  alGetSourcei(sourceID, AL_BUFFER, &buf);
  param.source_id = sourceID;
  param.buffer_id = buf;
}
void Sound::SetListener(glm::vec3 pos, glm::vec3 rotation) {
  glm::quat q = glm::angleAxis(rotation.z, glm::vec3(0, 0, -1));
  q = q * glm::angleAxis(rotation.y, glm::vec3(0, 1, 0));
  q = q * glm::angleAxis(rotation.x, glm::vec3(1, 0, 0));

  SoundSystem::set_listener(pos, q);
}
void Sound::DeleteSource() { SoundSystem::delete_source(param.source_id); }
bool Sound::IsValid() {
  return SoundSystem::get_buffers().contains(mName.data());
}

void Sound::Restart() {
  isPlaying = true;
  Stop();
  alSourcePlay(param.source_id);
}

void Sound::Stop(bool allowFadeOut /* true */) {
  isPlaying = false;
  alSourceStop(param.source_id);
}

void Sound::SetPaused(bool pause) {
  isPaused = pause;
  alSourcePause(param.source_id);
}

void Sound::SetVolume(float value) {
  volume = value;
  alSourcef(param.source_id, AL_GAIN, value);
}

void Sound::SetPitch(float value) {
  pitch = value;
  alSourcef(param.source_id, AL_PITCH, value);
}

void Sound::SetPosition(glm::vec3 pos) {
  this->pos = pos;
  alSource3f(param.source_id, AL_POSITION, pos.x, pos.y, pos.z);
}

bool Sound::GetPaused() { return isPaused; }

float Sound::GetVolume() { return volume; }

float Sound::GetPitch() { return pitch; }

const glm::vec3 &Sound::GetPosition() { return pos; }

std::string Sound::GetName() { return mName; }

} // namespace sinen
