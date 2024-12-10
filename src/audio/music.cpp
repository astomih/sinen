// std
#include <memory>
#include <string>
#include <unordered_map>

// internal
#include <audio/music.hpp>
#include <io/data_stream.hpp>

// external
#include <SDL_mixer.h>

namespace sinen {
Music::Music() {}

Music::~Music() {}

void Music::set_volume(int value) { ::Mix_VolumeMusic(value); }

void Music::play() { ::Mix_PlayMusic((::Mix_Music *)buffer, -1); }

void Music::stop() { ::Mix_HaltMusic(); }

void Music::load(std::string_view fileName) {
  auto rwops =
      (SDL_RWops *)DataStream::open_as_rwops(AssetType::Music, fileName);
  ::Mix_Music *m = ::Mix_LoadMUS_RW(rwops, 1);
  if (m) {
    buffer = (void *)m;
  }
}

void Music::unload() {
  if (buffer) {
    ::Mix_FreeMusic((::Mix_Music *)buffer);
    buffer = nullptr;
  }
}

} // namespace sinen
