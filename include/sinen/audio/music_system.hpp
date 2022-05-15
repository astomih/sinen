#pragma once
#include <memory>
#include <string>

namespace nen {
class music {
public:
  music();
  ~music();

  void play();

  void load(std::string_view fileName);
  void unload();

  void set_volume(int volume);

private:
  void *buffer;
};
} // namespace nen
