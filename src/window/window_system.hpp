#ifndef SINEN_WINDOW_SYSTEM_HPP
#define SINEN_WINDOW_SYSTEM_HPP
#include <SDL.h>
#include <math/vector2.hpp>
#include <memory>
#include <render/graphics_api.hpp>
#include <render/renderer.hpp>
#include <string>
#include <window/window_state.hpp>

namespace sinen {
class window_system {
public:
  static void initialize(const std::string &name, graphics_api api);
  static void shutdown();
  static void process_input();
  static SDL_Window *get_sdl_window() { return m_window; }
  static vector2 size() { return m_size; }
  static std::string name() { return m_name; }
  static const window_state &state() { return m_state; }

private:
  static vector2 m_size;
  static std::string m_name;
  static window_state m_state;
  static ::SDL_Window *m_window;
};
} // namespace sinen
#endif // SINEN_WINDOW_SYSTEM_HPP
