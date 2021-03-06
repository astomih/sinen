#pragma once
#include "../math/math.hpp"
#include "../math/vector3.hpp"
#include <cstdint>
#include <memory>
#include <string>

namespace nen {
struct sound_prameter {
  vector3 position;
  uint32_t source_id;
  uint32_t buffer_id;
};

class sound {
public:
  sound();
  void load(std::string_view file_name);
  void new_source();
  void delete_source();
  void play();
  bool is_valid();
  // Restart event from begining
  void restart();
  // Stop this event
  void stop(bool allowFadeOut = true);
  // Setters
  void set_paused(bool pause);
  void set_volume(float value);
  void set_pitch(float value);
  void set_position(vector3 pos);
  // Getters
  bool get_paused();
  float get_volume();
  float get_pitch();
  std::string get_name();
  const vector3 &get_position();

  void set_listener(vector3 pos, vector3 direction);

private:
  std::string mName;
  sound_prameter param;
  float volume = 1.f;
  float pitch = 1.f;
  vector3 pos;
  bool isPlaying = true;
  bool isPaused = false;
};
} // namespace nen
