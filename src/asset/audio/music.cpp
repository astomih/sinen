// std
#include <memory>
#include <string>
#include <unordered_map>

// internal
#include <asset/audio/music.hpp>
#include <core/io/asset_io.hpp>

// external
#include <SDL3_mixer/SDL_mixer.h>

namespace sinen {
Music::Music() {}

Music::~Music() {}

void Music::SetVolume(int value) { ::Mix_VolumeMusic(value); }

void Music::Play() { ::Mix_PlayMusic((::Mix_Music *)buffer, -1); }

void Music::Stop() { ::Mix_HaltMusic(); }

void Music::Load(std::string_view fileName) {
  auto rwops = (SDL_IOStream *)AssetIO::OpenAsRWOps(AssetType::Music, fileName);
  ::Mix_Music *m = ::Mix_LoadMUS_IO(rwops, 1);
  if (m) {
    buffer = (void *)m;
  }
}

void Music::Unload() {
  if (buffer) {
    ::Mix_FreeMusic((::Mix_Music *)buffer);
    buffer = nullptr;
  }
}

} // namespace sinen
