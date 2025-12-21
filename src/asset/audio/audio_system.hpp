#ifndef SINEN_SOUND_SYSTEM_HPP
#define SINEN_SOUND_SYSTEM_HPP
// internal
#include <asset/audio/sound.hpp>

// external
#include <miniaudio.h>

namespace sinen {
class AudioSystem {
public:
  static bool initialize();
  static void shutdown();

  static ma_engine *getEngine() { return &data.engine; }

private:
  struct Data {
    ma_engine engine;
    ma_resource_manager resouceManager;
  };
  inline static Data data;
};
} // namespace sinen
#endif // SINEN_SOUND_SYSTEM_HPP