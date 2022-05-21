#pragma once
#include <SDL.h>
#include <math/vector2.hpp>
#include <memory>
#include <render/graphics_api.hpp>
#include <render/renderer.hpp>
#include <string>
#include <window/window_state.hpp>

namespace nen {
class window_system {
public:
  window_system();
  void initialize(const std::string &name, graphics_api api);
  void ProcessInput();
  SDL_Window *GetSDLWindow() { return m_window; }
  vector2 Size() { return size; }

  vector2 size;
  std::string name;
  window_state state = window_state::ENTER;
  ::SDL_Window *m_window;
};
} // namespace nen