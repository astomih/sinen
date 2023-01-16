#ifndef SINEN_WINDOW_HPP
#define SINEN_WINDOW_HPP
#include "../math/vector2.hpp"
#include "window_state.hpp"
#include <string>

namespace sinen {
/**
 * @brief Window class
 *
 */
class window {
public:
  /**
   * @brief Get the window size
   *
   * @return vector2 size
   */
  static vector2 size();
  /**
   * @brief Get the window center
   *
   * @return vector2
   */
  static vector2 center();
  /**
   * @brief Set the Window size
   *
   * @param size window size
   */
  static void set_size(const vector2 &size);
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
  static void set_name(const std::string &name);
  /**
   * @brief Get the window name
   *
   * @return std::string name
   */
  static std::string name();
  /**
   * @brief Get the state of the window
   *
   * @return const window_state& state
   */
  static const window_state &state();
  /**
   * @brief Get the sdl window object
   *
   * @return const void* SDL_Window
   */
  static const void *get_sdl_window();
};
} // namespace sinen
#endif // !SINEN_WINDOW_HPP
