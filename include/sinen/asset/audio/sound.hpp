#ifndef SINEN_SOUND_HPP
#define SINEN_SOUND_HPP

#include <cstdint>
#include <memory>
#include <string>

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
   * @brief  Sound parameters
   *
   */
  struct Parameter {
    glm::vec3 position;
    uint32_t source_id;
    uint32_t buffer_id;
  };
  /**
   * @brief Construct a new sound object
   *
   */
  Sound();
  /**
   * @brief Load the sound data from file
   *
   * @param file_name File name
   */
  void Load(std::string_view file_name);
  /**
   * @brief Create the source
   *
   */
  void NewSource();
  /**
   * @brief Delete the source
   *
   */
  void DeleteSource();
  void Play();
  bool IsValid();
  // Restart event from begining
  void Restart();
  // Stop this event
  void Stop(bool allowFadeOut = true);
  // Setters
  void SetPaused(bool pause);
  void SetVolume(float value);
  void SetPitch(float value);
  void SetPosition(glm::vec3 pos);
  // Getters
  bool GetPaused();
  float GetVolume();
  float GetPitch();
  std::string GetName();
  const glm::vec3 &GetPosition();

  void SetListener(glm::vec3 pos, glm::vec3 direction);

private:
  std::string mName;
  Parameter param;
  float volume = 1.f;
  float pitch = 1.f;
  glm::vec3 pos;
  bool isPlaying = true;
  bool isPaused = false;
};
} // namespace sinen
#endif // !SINEN_SOUND_HPP