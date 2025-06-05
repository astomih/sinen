#include <SDL3/SDL.h>
#include <core/time/time.hpp>

namespace sinen {
float Time::seconds() { return static_cast<float>(SDL_GetTicks() / 1000.f); }

uint32_t Time::milli() { return SDL_GetTicks(); }
} // namespace sinen
