#include "../../main_system.hpp"
#include <SDL3/SDL.h>
#include <core/time/time.hpp>

namespace sinen {
float Time::seconds() { return static_cast<float>(SDL_GetTicks() / 1000.f); }

uint32_t Time::milli() { return SDL_GetTicks(); }
float Time::deltaTime() { return MainSystem::deltaTime(); }
} // namespace sinen
