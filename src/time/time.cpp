#include <SDL.h>
#include <time/time.hpp>

namespace sinen {
float time::seconds() { return static_cast<float>(SDL_GetTicks() / 1000.f); }

uint32_t time::milli() { return SDL_GetTicks(); }
} // namespace sinen
