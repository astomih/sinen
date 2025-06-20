#include "window_system.hpp"
#include <SDL3/SDL.h>
#include <core/io/file.hpp>
#include <core/io/json.hpp>
#include <platform/input/keyboard.hpp>
#include <platform/window/window.hpp>

namespace sinen {
glm::vec2 WindowSystem::m_size = glm::vec2(1280.f, 720.f);
std::string WindowSystem::m_name = "";
::SDL_Window *WindowSystem::m_window = nullptr;
bool WindowSystem::m_resized = false;
const void *Window::GetSDLWindow() { return WindowSystem::get_sdl_window(); }
glm::vec2 Window::Size() { return WindowSystem::size(); }
glm::vec2 Window::Half() { return WindowSystem::half(); }
void Window::Resize(const glm::vec2 &size) { WindowSystem::resize(size); }
void Window::SetFullscreen(bool fullscreen) {
  WindowSystem::set_fullscreen(fullscreen);
}
void Window::Rename(const std::string &name) { WindowSystem::rename(name); }
std::string Window::GetName() { return WindowSystem::name(); }
bool Window::Resized() { return WindowSystem::resized(); }

void WindowSystem::initialize(const std::string &name) {
  m_name = name;

  // Load settings from settings.json
  {
    File f;
    if (f.Open("settings.json", File::Mode::r)) {

      void *buffer = calloc(f.Size() + 10, 1);
      f.Read(buffer, f.Size(), 1);
      f.close();
      Json j;
      j.parse((char *)buffer);
      m_size.x = j["WindowWidth"].get_float();
      m_size.y = j["WindowHeight"].get_float();
    } else {
      f.close();
      m_size.x = 1280;
      m_size.y = 720;
    }
  }

  m_window = SDL_CreateWindow(
      std::string(name).c_str(), static_cast<int>(m_size.x),
      static_cast<int>(m_size.y), SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
  SDL_StartTextInput(m_window);
}

void WindowSystem::shutdown() {
  SDL_DestroyWindow(m_window);
  m_window = nullptr;
}
void WindowSystem::resize(const glm::vec2 &size) {
  m_size = size;
  SDL_SetWindowSize(m_window, static_cast<int>(m_size.x),
                    static_cast<int>(m_size.y));
}
void WindowSystem::set_fullscreen(bool fullscreen) {
  SDL_SetWindowFullscreen(m_window, fullscreen);
}
void WindowSystem::rename(const std::string &name) {
  m_name = name;
  SDL_SetWindowTitle(m_window, m_name.c_str());
}
void WindowSystem::prepare_frame() { m_resized = false; }
void WindowSystem::process_input(SDL_Event &event) {
  int x, y;
  SDL_GetWindowSize(m_window, &x, &y);
  m_size.x = static_cast<float>(x);
  m_size.y = static_cast<float>(y);
  if (event.window.type == SDL_EventType::SDL_EVENT_WINDOW_RESIZED) {
    m_resized = true;
  }
  if (Keyboard::IsPressed(Keyboard::code::F11)) {
    static bool fullscreen = false;
    fullscreen = !fullscreen;
    set_fullscreen(fullscreen);
  }
}
} // namespace sinen
