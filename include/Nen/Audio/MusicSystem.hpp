#pragma once
#include <memory>
#include <string>

namespace nen {
class music_player {
public:
  music_player();
  ~music_player();

  void PlayMusic(std::string_view name);

  void LoadMusicFromFile(std::string_view fileName);
  void UnloadAllMusic();
  void UnloadMusic(std::string_view name);

  void set_volume(int volume);

  std::string_view name;

protected:
  friend class event;

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};
} // namespace nen
