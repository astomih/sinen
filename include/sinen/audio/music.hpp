#ifndef SINEN_MUSIC_HPP
#define SINEN_MUSIC_HPP
#include <memory>
#include <string>

namespace sinen {
class music {
public:
  music();
  ~music();

  void play();
  void stop();

  void load(std::string_view fileName);
  void unload();

  void set_volume(int volume);

private:
  void *buffer;
};
} // namespace sinen
#endif // !SINEN_MUSIC_HPP