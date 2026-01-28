#ifndef SINEN_AUDIO_HPP
#define SINEN_AUDIO_HPP
// internal
#include <asset/audio/sound.hpp>

namespace sinen {
class Audio {
public:
  static bool initialize();
  static void shutdown();

  static void *getEngine();
};
} // namespace sinen
#endif // SINEN_AUDIO_HPP