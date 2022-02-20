#pragma once
#include "../Math/Math.hpp"
#include "../Math/Vector3.hpp"
#include <cstdint>
#include <memory>
#include <string>

namespace nen {
struct sound_prameter {
  vector3 position;
  uint32_t source_id;
  uint32_t buffer_id;
};

class sound_event {
public:
  sound_event(class sound_system &audiosystem, std::string_view name,
              uint32_t sourceID = 0);
  bool IsValid();
  // Restart event from begining
  void Restart();
  // Stop this event
  void Stop(bool allowFadeOut = true);
  // Setters
  void SetPaused(bool pause);
  void SetVolume(float value);
  void SetPitch(float value);
  void SetPosition(vector3 pos);
  // Getters
  bool GetPaused();
  float GetVolume();
  float GetPitch();
  std::string GetName();
  const vector3 &GetPosition();

private:
  sound_system &audiosys;
  std::string mName;
  sound_prameter param;
  float volume = 1.f;
  float pitch = 1.f;
  vector3 pos;
  bool isPlaying = true;
  bool isPaused = false;
};
} // namespace nen
