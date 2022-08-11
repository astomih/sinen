#ifndef SINEN_WINDOW_HPP
#define SINEN_WINDOW_HPP
#include "../math/vector2.hpp"
#include "window_state.hpp"
#include <string>

namespace sinen {
class window {
public:
  window();
  ~window();

  void *GetSDLWindow();

  vector2 size();
  std::string name();

  const window_state &state();
};
} // namespace sinen
#endif // !SINEN_WINDOW_HPP
