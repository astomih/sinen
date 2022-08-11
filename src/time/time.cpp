#include <SDL.h>
#include <time/time.hpp>

namespace sinen {
float time::get_ticks_as_seconds() {
  return static_cast<float>(SDL_GetTicks() / 1000.f);
}

uint32_t time::get_ticks_as_milli_seconds() { return SDL_GetTicks(); }
} // namespace nen
