#pragma once
#include "../Math/Math.hpp"
#include "../Math/Quaternion.hpp"
#include "../Math/Vector3.hpp"
#include "SoundEvent.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace nen {
class sound_system {
public:
  sound_system();
  ~sound_system();

  bool Initialize();
  void Shutdown();
  void Update(float deltaTime);

  sound_event PlayEvent(std::string_view name, uint32_t sourceID = 0);

  void LoadAudioFile(std::string_view fileName);
  void UnloadAudioFile(std::string_view fileName);

  uint32_t NewSource(std::string_view name);
  void DeleteSource(uint32_t sourceID);

  // For positional audio
  void SetListener(const vector3 &pos, const quaternion &direction);

  std::unordered_map<std::string, uint32_t> &get_buffers() { return buffers; }

private:
  std::unordered_map<std::string, uint32_t> buffers;
  void *device;
  void *context;
};
} // namespace nen
