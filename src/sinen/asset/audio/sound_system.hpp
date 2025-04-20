#ifndef SINEN_SOUND_SYSTEM_HPP
#define SINEN_SOUND_SYSTEM_HPP
// std
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

// internal
#include <asset/audio/sound.hpp>

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
  static void set_listener(const glm::vec3 &pos, const glm::quat &direction);
  static std::unordered_map<std::string, uint32_t> &get_buffers() {
    return buffers;
  }

private:
  static std::unordered_map<std::string, uint32_t> buffers;
  static void *device;
  static void *context;
};
} // namespace sinen
#endif // SINEN_SOUND_SYSTEM_HPP