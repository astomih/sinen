#ifndef SINEN_WINDOW_SYSTEM_HPP
#define SINEN_WINDOW_SYSTEM_HPP
#include <SDL.h>
#include <math/vector2.hpp>
#include <memory>
#include <render/renderer.hpp>
#include <string>

namespace sinen {
class window_system {
public:
  static void initialize(const std::string &name);
  static void shutdown();
  static void prepare_frame();
  static void process_input(SDL_Event &event);
  static SDL_Window *get_sdl_window() { return m_window; }
  static void resize(const vector2 &size);
  static void set_fullscreen(bool fullscreen);
  static void rename(const std::string &name);

  static vector2 size() { return m_size; }
  static bool resized() { return m_resized; }
  static vector2 center() { return vector2(m_size.x / 2.0, m_size.y / 2.0); }
  static std::string name() { return m_name; }

private:
  static bool m_resized;
  static vector2 m_size;
  static std::string m_name;
  static ::SDL_Window *m_window;
};
} // namespace sinen
#endif // SINEN_WINDOW_SYSTEM_HPP
