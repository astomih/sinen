#ifndef SINEN_SOUND_HPP
#define SINEN_SOUND_HPP
#include <core/buffer/buffer.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <math/vector.hpp>

#include <miniaudio.h>

namespace sinen {
/**
 * @brief Sound class
 *
 */
class Sound {
public:
  /**
   * @brief Construct a new sound object
   *
   */
  Sound();

  static constexpr const char *metaTableName() { return "sn.Sound"; }
  /**
   * @brief Destruct sound object
   */
  ~Sound();
  /**
   * @brief Load the sound data from file
   *
   * @param fileName File name
   */
  void load(StringView fileName);
  void load(const Buffer &buffer);
  void play();
  // Restart event from begining
  void restart();
  // Stop this event
  void stop();
  // Setters
  void setLooping(bool looping);
  void setVolume(float value);
  void setPitch(float value);
  void setPosition(const Vec3 &pos);
  void setDirection(const Vec3 &dir);
  // Getters
  bool isPlaying() const;
  bool isLooping() const;
  float getVolume() const;
  float getPitch() const;
  Vec3 getPosition() const;
  Vec3 getDirection() const;

private:
  ma_sound sound;
};
} // namespace sinen
#endif // !SINEN_SOUND_HPP
