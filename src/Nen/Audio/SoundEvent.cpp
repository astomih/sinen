#include <AL/al.h>
#include <AL/alc.h>
#include <Nen.hpp>
namespace nen {
sound_event::sound_event(sound_system &audiosystem, std::string_view name,
                         uint32_t sourceID)
    : audiosys(audiosystem), mName(name) {
  ALint buf;
  alGetSourcei(sourceID, AL_BUFFER, &buf);
  alSourcePlay(sourceID);
  param.source_id = sourceID;
  param.buffer_id = buf;
}
bool sound_event::IsValid() {
  return audiosys.get_buffers().contains(mName.data());
}

void sound_event::Restart() {
  isPlaying = true;
  Stop();
  alSourcePlay(param.source_id);
}

void sound_event::Stop(bool allowFadeOut /* true */) {
  isPlaying = false;
  alSourceStop(param.source_id);
}

void sound_event::SetPaused(bool pause) {
  isPaused = pause;
  alSourcePause(param.source_id);
}

void sound_event::SetVolume(float value) {
  volume = value;
  alSourcef(param.source_id, AL_GAIN, value);
}

void sound_event::SetPitch(float value) {
  pitch = value;
  alSourcef(param.source_id, AL_PITCH, value);
}

void sound_event::SetPosition(vector3 pos) {
  this->pos = pos;
  alSource3f(param.source_id, AL_POSITION, pos.x, pos.y, pos.z);
}

bool sound_event::GetPaused() { return isPaused; }

float sound_event::GetVolume() { return volume; }

float sound_event::GetPitch() { return pitch; }

const vector3 &sound_event::GetPosition() { return pos; }

std::string sound_event::GetName() { return mName; }

} // namespace nen
