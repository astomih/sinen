#ifndef SINEN_WINDOW_HPP
#define SINEN_WINDOW_HPP
#include <glm/vec2.hpp>

#include <string>

namespace sinen {
/**
 * @brief Window class
 *
 */
class Window {
public:
  /**
   * @brief Get the window size
   *
   * @return vector2 size
   */
  static glm::vec2 size();
  /**
   * @brief Get the window half size
   *
   * @return vector2
   */
  static glm::vec2 half();
  /**
   * @brief Set the Window size
   *
   * @param size window size
   */
  static void resize(const glm::vec2 &size);
  /**
   * @brief Set the fullscreen
   *
   * @param fullscreen
   */
  static void set_fullscreen(bool fullscreen);
  /**
   * @brief Set the name
   *
   * @param name
   */
  static void rename(const std::string &name);
  /**
   * @brief Get the window name
   *
   * @return std::string name
   */
  static std::string name();
  /**
   * @brief Check if the window is resized
   *
   * @return true Resized window
   * @return false Not resized window
   */
  static bool resized();
  /**
   * @brief Get the sdl window object
   *
   * @return const void* SDL_Window
   */
  static const void *get_sdl_window();
};
} // namespace sinen
#endif // !SINEN_WINDOW_HPP
