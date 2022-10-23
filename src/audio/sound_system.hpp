#pragma once
#include <audio/sound.hpp>
#include <math/math.hpp>
#include <math/quaternion.hpp>
#include <math/vector3.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace sinen {
class sound_system {
public:
  sound_system();
  ~sound_system();

  bool initialize();
  void shutdown();
  void update(float deltaTime);

  void load(std::string_view fileName);
  void unload(std::string_view fileName);

  uint32_t new_source(std::string_view name);
  void delete_source(uint32_t sourceID);

  // For positional audio
  void set_listener(const vector3 &pos, const quaternion &direction);

  std::unordered_map<std::string, uint32_t> &get_buffers() { return buffers; }

private:
  std::unordered_map<std::string, uint32_t> buffers;
  void *device;
  void *context;
};
} // namespace sinen
