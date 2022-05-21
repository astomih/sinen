#include "../event/current_event.hpp"
#include "../manager/get_system.hpp"
#include "utility/singleton.hpp"
#include "window_system.hpp"
#include <SDL.h>
#include <window/window.hpp>

namespace nen {
window_system::window_system() : size(vector2(1280.f, 720.f)) {}
window::~window() = default;

void *window::GetSDLWindow() { return get_window().m_window; }

void window_system::initialize(const std::string &name, graphics_api api) {
  this->name = name;

#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  switch (api) {
  case graphics_api::Vulkan: {
    get_window().m_window = SDL_CreateWindow(
        std::string(name).c_str(), SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, static_cast<int>(size.x),
        static_cast<int>(size.y), SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);

    break;
  }
  case graphics_api::OpenGL: {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    m_window = SDL_CreateWindow(
        std::string(name).c_str(), SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, static_cast<int>(size.x),
        static_cast<int>(size.y), SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    break;
  }
  default:
    break;
  }
#endif
#if defined(EMSCRIPTEN) || defined(MOBILE)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  m_window = SDL_CreateWindow(
      std::string(name).c_str(), SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, static_cast<int>(size.x),
      static_cast<int>(size.y), SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
#endif
}

void window_system::ProcessInput() {
  int x, y;
  SDL_GetWindowSize(m_window, &x, &y);
  this->size.x = static_cast<float>(x);
  this->size.y = static_cast<float>(y);
  if (current_event_handle::current_event.type == SDL_WINDOWEVENT) {
    state = static_cast<window_state>(
        current_event_handle::current_event.window.event);
  }
}
} // namespace nen