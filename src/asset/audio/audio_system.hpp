#ifndef SINEN_SOUND_SYSTEM_HPP
#define SINEN_SOUND_SYSTEM_HPP
// std
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

// internal
#include <asset/audio/sound.hpp>

// external
#include <miniaudio.h>

namespace sinen {
class AudioSystem {
public:
  static bool initialize();
  static void shutdown();
  static void update(float deltaTime);
  static void load(ma_sound *sound, std::string_view fileName);
  static void loadFromPath(std::string_view path);
  static void unload(std::string_view fileName);
  static uint32_t new_source(std::string_view name);
  static void deleteSource(uint32_t sourceID);
  // For positional audio
  static void setListener(const glm::vec3 &pos, const glm::quat &direction);
  static std::unordered_map<std::string, uint32_t> &get_buffers() {
    return buffers;
  }

private:
  static std::unordered_map<std::string, uint32_t> buffers;
  struct Data {
    ma_engine engine;
    ma_resource_manager resouceManager;
  };
  inline static Data data;
};
} // namespace sinen
#endif // SINEN_SOUND_SYSTEM_HPP