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

  static const void *get_sdl_window();

  static vector2 size();
  static std::string name();

  static const window_state &state();
};
} // namespace sinen
#endif // !SINEN_WINDOW_HPP
