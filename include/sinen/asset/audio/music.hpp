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
  void play();
  /**
   * @brief Stop the music
   *
   */
  void stop();
  /**
   * @brief Load the music from file
   *
   * @param fileName file name
   */
  void load(std::string_view fileName);
  /**
   * @brief Unload the music
   *
   */
  void unload();
  /**
   * @brief Set the volume object
   *
   * @param volume volume 0.0 - 1.0
   */
  void set_volume(int volume);

private:
  void *buffer;
};
} // namespace sinen
#endif // !SINEN_MUSIC_HPP