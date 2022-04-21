#pragma once
#include "../manager/manager.hpp"

#include "../math/vector2.hpp"
#include "../render/graphics_api.hpp"
#include "../render/renderer.hpp"
#include "window_state.hpp"
#include <memory>
#include <string>

namespace nen {
class window {
public:
  window();
  ~window();

  void Initialize(const std::string &name, graphics_api api);

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