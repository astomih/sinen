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
  static glm::vec2 Size();
  /**
   * @brief Get the window half size
   *
   * @return vector2
   */
  static glm::vec2 Half();
  /**
   * @brief Set the Window size
   *
   * @param size window size
   */
  static void Resize(const glm::vec2 &size);
  /**
   * @brief Set the fullscreen
   *
   * @param fullscreen
   */
  static void SetFullscreen(bool fullscreen);
  /**
   * @brief Set the name
   *
   * @param name
   */
  static void Rename(const std::string &name);
  /**
   * @brief Get the window name
   *
   * @return std::string name
   */
  static std::string GetName();
  /**
   * @brief Check if the window is resized
   *
   * @return true Resized window
   * @return false Not resized window
   */
  static bool Resized();
  /**
   * @brief Get the sdl window object
   *
   * @return const void* SDL_Window
   */
  static const void *GetSDLWindow();
};
} // namespace sinen
#endif // !SINEN_WINDOW_HPP
