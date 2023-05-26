// internal
#include "sound_system.hpp"
#include <audio/music.hpp>
#include <audio/sound.hpp>

// external
#define AL_LIBTYPE_STATIC
#include <AL/al.h>
#include <AL/alc.h>

namespace sinen {
sound::sound() {}
void sound::load(std::string_view file_name) {
  sound_system::load(file_name);
  auto sourceID = sound_system::new_source(file_name);
  ALint buf;
  alGetSourcei(sourceID, AL_BUFFER, &buf);
  param.source_id = sourceID;
  param.buffer_id = buf;
  mName = file_name.data();
}
void sound::play() { alSourcePlay(param.source_id); }
void sound::new_source() {
  auto sourceID = sound_system::new_source(mName);
  ALint buf;
  alGetSourcei(sourceID, AL_BUFFER, &buf);
  param.source_id = sourceID;
  param.buffer_id = buf;
}
void sound::set_listener(vector3 pos, vector3 rotation) {
  quaternion q(vector3::neg_unit_z, rotation.z);
  q = quaternion::concatenate(q, quaternion(vector3::unit_y, rotation.y));
  q = quaternion::concatenate(q, quaternion(vector3::unit_x, rotation.x));
  sound_system::set_listener(pos, q);
}
void sound::delete_source() { sound_system::delete_source(param.source_id); }
bool sound::is_valid() {
  return sound_system::get_buffers().contains(mName.data());
}

void sound::restart() {
  isPlaying = true;
  stop();
  alSourcePlay(param.source_id);
}

void sound::stop(bool allowFadeOut /* true */) {
  isPlaying = false;
  alSourceStop(param.source_id);
}

void sound::set_paused(bool pause) {
  isPaused = pause;
  alSourcePause(param.source_id);
}

void sound::set_volume(float value) {
  volume = value;
  alSourcef(param.source_id, AL_GAIN, value);
}

void sound::set_pitch(float value) {
  pitch = value;
  alSourcef(param.source_id, AL_PITCH, value);
}

void sound::set_position(vector3 pos) {
  this->pos = pos;
  alSource3f(param.source_id, AL_POSITION, pos.x, pos.y, pos.z);
}

bool sound::get_paused() { return isPaused; }

float sound::get_volume() { return volume; }

float sound::get_pitch() { return pitch; }

const vector3 &sound::get_position() { return pos; }

std::string sound::get_name() { return mName; }

} // namespace sinen
