// internal
#include "sound_system.hpp"
#include <audio/music.hpp>
#include <audio/sound.hpp>

// external
#define AL_LIBTYPE_STATIC
#include <AL/al.h>
#include <AL/alc.h>

namespace sinen {
Sound::Sound() {}
void Sound::load(std::string_view file_name) {
  sound_system::load(file_name);
  auto sourceID = sound_system::new_source(file_name);
  ALint buf;
  alGetSourcei(sourceID, AL_BUFFER, &buf);
  param.source_id = sourceID;
  param.buffer_id = buf;
  mName = file_name.data();
}
void Sound::play() { alSourcePlay(param.source_id); }
void Sound::new_source() {
  auto sourceID = sound_system::new_source(mName);
  ALint buf;
  alGetSourcei(sourceID, AL_BUFFER, &buf);
  param.source_id = sourceID;
  param.buffer_id = buf;
}
void Sound::set_listener(Vector3 pos, Vector3 rotation) {
  Quaternion q(Vector3::neg_unit_z, rotation.z);
  q = Quaternion::concatenate(q, Quaternion(Vector3::unit_y, rotation.y));
  q = Quaternion::concatenate(q, Quaternion(Vector3::unit_x, rotation.x));
  sound_system::set_listener(pos, q);
}
void Sound::delete_source() { sound_system::delete_source(param.source_id); }
bool Sound::is_valid() {
  return sound_system::get_buffers().contains(mName.data());
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

void Sound::set_paused(bool pause) {
  isPaused = pause;
  alSourcePause(param.source_id);
}

void Sound::set_volume(float value) {
  volume = value;
  alSourcef(param.source_id, AL_GAIN, value);
}

void Sound::set_pitch(float value) {
  pitch = value;
  alSourcef(param.source_id, AL_PITCH, value);
}

void Sound::set_position(Vector3 pos) {
  this->pos = pos;
  alSource3f(param.source_id, AL_POSITION, pos.x, pos.y, pos.z);
}

bool Sound::get_paused() { return isPaused; }

float Sound::get_volume() { return volume; }

float Sound::get_pitch() { return pitch; }

const Vector3 &Sound::get_position() { return pos; }

std::string Sound::get_name() { return mName; }

} // namespace sinen
