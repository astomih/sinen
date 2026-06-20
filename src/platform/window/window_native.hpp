#ifndef SINEN_WINDOW_NATIVE_HPP
#define SINEN_WINDOW_NATIVE_HPP
#include <SDL3/SDL_video.h>
namespace sinen {
class WindowNative {
public:
  static SDL_Window *getWindow();
};
} // namespace sinen
#endif
