#pragma once
#include <memory>
#include <string>

namespace nen {
class music {
public:
  music();
  ~music();

  void PlayMusic();

  void LoadMusicFromFile(std::string_view fileName);
  void UnloadMusic();

  void set_volume(int volume);

private:
  void *buffer;
};
} // namespace nen
