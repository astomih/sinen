#include <memory>
#include <string>
#include <unordered_map>

#include <SDL_mixer.h>

#include <audio/music_system.hpp>
#include <io/dstream.hpp>

namespace nen {
music::music() {}

music::~music() {}

void music::set_volume(int value) { ::Mix_VolumeMusic(value); }

void music::PlayMusic() { ::Mix_PlayMusic((::Mix_Music *)buffer, -1); }

void music::LoadMusicFromFile(std::string_view fileName) {
  auto rwops = (SDL_RWops *)dstream::open_as_rwops(asset_type::Music, fileName);
  ::Mix_Music *m = ::Mix_LoadMUS_RW(rwops, 1);
  if (m) {
    buffer = (void *)m;
  }
}

void music::UnloadMusic() {
  if (buffer) {
    ::Mix_FreeMusic((::Mix_Music *)buffer);
    buffer = nullptr;
  }
}

} // namespace nen