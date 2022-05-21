#pragma once
#include "../math/vector2.hpp"
#include "window_state.hpp"
#include <string>

namespace nen {
class window {
public:
  window();
  ~window();

  void *GetSDLWindow();

  vector2 size();
  std::string name();

  const window_state &state();
};
} // namespace nen