#include "Audio/MusicSystem.hpp"
#include "sol/raii.hpp"
#include <Nen.hpp>
#include <SDL_mixer.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace nen {
class music_player::Impl {
public:
  Impl() = default;
  ~Impl() = default;
  std::unordered_map<std::string, ::Mix_Music *> buffer;
};
music_player::music_player() {
  impl = std::make_unique<Impl>();
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
}

music_player::~music_player() {
  UnloadAllMusic();
  Mix_CloseAudio();
}

void music_player::set_volume(int value) { ::Mix_VolumeMusic(value); }

void music_player::PlayMusic(std::string_view name) {
  for (auto &i : impl->buffer) {
    if (i.first == name) {
      ::Mix_PlayMusic(i.second, -1);
    }
  }
}

void music_player::LoadMusicFromFile(std::string_view fileName) {
  auto rwops =
      (SDL_RWops *)asset_reader::LoadAsRWops(asset_type::Music, fileName);
  ::Mix_Music *music = ::Mix_LoadMUS_RW(rwops, 1);
  if (music) {
    impl->buffer.emplace(std::string(fileName), music);
  }
}

void music_player::UnloadAllMusic() {
  for (auto i = impl->buffer.begin(); i != impl->buffer.end();) {
    Mix_FreeMusic(i->second);
    i = impl->buffer.erase(i);
    if (i != impl->buffer.end())
      i++;
  }
}

void music_player::UnloadMusic(std::string_view fileName) {
  for (auto i = impl->buffer.begin(); i != impl->buffer.end();) {
    if (i->first == fileName) {
      Mix_FreeMusic(i->second);
      impl->buffer.erase(i);
      break;
    }
    i++;
  }
}

} // namespace nen