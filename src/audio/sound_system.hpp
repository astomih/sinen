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
  static bool initialize();
  static void shutdown();
  static void update(float deltaTime);
  static void load(std::string_view fileName);
  static void unload(std::string_view fileName);
  static uint32_t new_source(std::string_view name);
  static void delete_source(uint32_t sourceID);
  // For positional audio
  static void set_listener(const vector3 &pos, const quaternion &direction);
  static std::unordered_map<std::string, uint32_t> &get_buffers() {
    return buffers;
  }

private:
  static std::unordered_map<std::string, uint32_t> buffers;
  static void *device;
  static void *context;
};
} // namespace sinen
