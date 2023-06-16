#ifndef SINEN_SOUND_HPP
#define SINEN_SOUND_HPP

#include "../math/math.hpp"
#include "../math/vector3.hpp"
#include <cstdint>
#include <memory>
#include <string>

namespace sinen {
/**
 * @brief Sound class
 *
 */
class sound {
public:
  /**
   * @brief  Sound parameters
   *
   */
  struct parameter {
    vector3 position;
    uint32_t source_id;
    uint32_t buffer_id;
  };
  /**
   * @brief Construct a new sound object
   *
   */
  sound();
  /**
   * @brief Load the sound data from file
   *
   * @param file_name File name
   */
  void load(std::string_view file_name);
  /**
   * @brief Create the source
   *
   */
  void new_source();
  /**
   * @brief Delete the source
   *
   */
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
  parameter param;
  float volume = 1.f;
  float pitch = 1.f;
  vector3 pos;
  bool isPlaying = true;
  bool isPaused = false;
};
} // namespace sinen
#endif // !SINEN_SOUND_HPP