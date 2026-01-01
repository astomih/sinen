#ifndef SINEN_SOUND_HPP
#define SINEN_SOUND_HPP
#include <core/buffer/buffer.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <math/vector.hpp>

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
  void play() const;
  // Restart event from begining
  void restart() const;
  // Stop this event
  void stop() const;
  // Setters
  void setLooping(bool looping) const;
  void setVolume(float value) const;
  void setPitch(float value) const;
  void setPosition(const Vec3 &pos) const;
  void setDirection(const Vec3 &dir) const;
  // Getters
  bool isPlaying() const;
  bool isLooping() const;
  float getVolume() const;
  float getPitch() const;
  Vec3 getPosition() const;
  Vec3 getDirection() const;

private:
  struct Data;
  UniquePtr<Data> data;
};
} // namespace sinen
#endif // !SINEN_SOUND_HPP