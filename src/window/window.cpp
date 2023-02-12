#include "window_system.hpp"
#include <SDL.h>
#include <input/keyboard.hpp>
#include <io/file.hpp>
#include <io/json.hpp>
#include <window/window.hpp>

namespace sinen {
vector2 window_system::m_size = vector2(1280.f, 720.f);
std::string window_system::m_name = "Sinen Engine";
::SDL_Window *window_system::m_window = nullptr;
bool window_system::m_resized = false;
const void *window::get_sdl_window() { return window_system::get_sdl_window(); }
vector2 window::size() { return window_system::size(); }
vector2 window::center() { return window_system::center(); }
void window::resize(const vector2 &size) { window_system::resize(size); }
void window::set_fullscreen(bool fullscreen) {
  window_system::set_fullscreen(fullscreen);
}
void window::rename(const std::string &name) { window_system::rename(name); }
std::string window::name() { return window_system::name(); }
bool window::resized() { return window_system::resized(); }

void window_system::initialize(const std::string &name, graphics_api api) {
  m_name = name;

#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  {
    file f;
    f.open("settings.json", file::mode::r);
    void *buffer = calloc(f.size() + 10, 1);
    f.read(buffer, f.size(), 1);
    f.close();
    json j;
    j.parse((char *)buffer);
    m_size.x = j["WindowWidth"].get_float();
    m_size.y = j["WindowHeight"].get_float();
  }

  switch (api) {
  case graphics_api::Vulkan: {
    m_window = SDL_CreateWindow(
        std::string(name).c_str(), SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, static_cast<int>(m_size.x),
        static_cast<int>(m_size.y), SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    break;
  }
  default:
    break;
  }
#endif
}

void window_system::shutdown() {
  SDL_DestroyWindow(m_window);
  m_window = nullptr;
}
void window_system::resize(const vector2 &size) {
  m_size = size;
  SDL_SetWindowSize(m_window, static_cast<int>(m_size.x),
                    static_cast<int>(m_size.y));
}
void window_system::set_fullscreen(bool fullscreen) {
  SDL_SetWindowFullscreen(m_window,
                          fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}
void window_system::rename(const std::string &name) {
  m_name = name;
  SDL_SetWindowTitle(m_window, m_name.c_str());
}
void window_system::prepare_frame() { m_resized = false; }
void window_system::process_input(SDL_Event &event) {
  int x, y;
  SDL_GetWindowSize(m_window, &x, &y);
  m_size.x = static_cast<float>(x);
  m_size.y = static_cast<float>(y);
  if (event.type == SDL_WINDOWEVENT) {
    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
      m_resized = true;
    }
  }
  if (keyboard::is_pressed(keyboard::code::F11)) {
    static bool fullscreen = false;
    fullscreen = !fullscreen;
    set_fullscreen(fullscreen);
  }
}
} // namespace sinen
