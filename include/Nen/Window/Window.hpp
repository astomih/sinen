#pragma once
#include "../Math/Vector2.hpp"
#include "../Render/Renderer.hpp"
#include "WindowState.hpp"
#include <memory>
#include <string>

namespace nen {
class window {
public:
  window();
  ~window();

  void Initialize(const vector2 &size, const std::string &name,
                  graphics_api api);

  void *GetSDLWindow();

  vector2 Size() { return size; }
  std::string Name() { return name; }

  void ProcessInput();
  const window_state &GetState() { return state; }

private:
  vector2 size;
  std::string name;
  class Impl;
  std::unique_ptr<Impl> impl;
  window_state state = window_state::ENTER;
};
} // namespace nen