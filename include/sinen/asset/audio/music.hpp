#ifndef SINEN_MUSIC_HPP
#define SINEN_MUSIC_HPP
#include <memory>
#include <string>

namespace sinen {
/**
 * @brief Music class
 * @details Long sound data
 *
 */
class Music {
public:
  /**
   * @brief Construct a new music object
   *
   */
  Music();
  /**
   * @brief Destroy the music object
   *
   */
  ~Music();
  /**
   * @brief Play the music
   *
   */
  void Play();
  /**
   * @brief Stop the music
   *
   */
  void Stop();
  /**
   * @brief Load the music from file
   *
   * @param fileName file name
   */
  void Load(std::string_view fileName);
  /**
   * @brief Unload the music
   *
   */
  void Unload();
  /**
   * @brief Set the volume object
   *
   * @param volume volume 0.0 - 1.0
   */
  void SetVolume(int volume);

private:
  void *buffer;
};
} // namespace sinen
#endif // !SINEN_MUSIC_HPP