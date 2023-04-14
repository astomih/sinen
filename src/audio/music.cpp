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
music::music() {}

music::~music() {}

void music::set_volume(int value) { ::Mix_VolumeMusic(value); }

void music::play() { ::Mix_PlayMusic((::Mix_Music *)buffer, -1); }

void music::stop() { ::Mix_HaltMusic(); }

void music::load(std::string_view fileName) {
  auto rwops =
      (SDL_RWops *)data_stream::open_as_rwops(asset_type::Music, fileName);
  ::Mix_Music *m = ::Mix_LoadMUS_RW(rwops, 1);
  if (m) {
    buffer = (void *)m;
  }
}

void music::unload() {
  if (buffer) {
    ::Mix_FreeMusic((::Mix_Music *)buffer);
    buffer = nullptr;
  }
}

} // namespace sinen
