#ifndef SINEN_SOUND_HPP
#define SINEN_SOUND_HPP
#include <core/data/ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>

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
  void load(std::string_view fileName);
  void loadFromPath(std::string_view path);
  void play() const;
  // Restart event from begining
  void restart() const;
  // Stop this event
  void stop() const;
  // Setters
  void setLooping(bool looping) const;
  void setVolume(float value) const;
  void setPitch(float value) const;
  void setPosition(const glm::vec3 &pos) const;
  void setDirection(const glm::vec3 &dir) const;
  // Getters
  bool isPlaying() const;
  bool isLooping() const;
  float getVolume() const;
  float getPitch() const;
  glm::vec3 getPosition() const;
  glm::vec3 getDirection() const;

private:
  struct Data;
  UniquePtr<Data> data;
};
} // namespace sinen
#endif // !SINEN_SOUND_HPP