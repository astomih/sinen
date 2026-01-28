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
  static constexpr const char *metaTableName() { return "sn.Sound"; }
  static Ptr<Sound> create();

  virtual ~Sound() = default;
  virtual void load(StringView fileName) = 0;
  virtual void load(const Buffer &buffer) = 0;
  virtual void play() = 0;
  virtual void restart() = 0;
  virtual void stop() = 0;
  virtual void setLooping(bool looping) = 0;
  virtual void setVolume(float value) = 0;
  virtual void setPitch(float value) = 0;
  virtual void setPosition(const Vec3 &pos) = 0;
  virtual void setDirection(const Vec3 &dir) = 0;
  virtual bool isPlaying() const = 0;
  virtual bool isLooping() const = 0;
  virtual float getVolume() const = 0;
  virtual float getPitch() const = 0;
  virtual Vec3 getPosition() const = 0;
  virtual Vec3 getDirection() const = 0;
};
} // namespace sinen
#endif // !SINEN_SOUND_HPP
