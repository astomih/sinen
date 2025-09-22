#include "../scene/scene_system.hpp"
#include <SDL3/SDL.h>
#include <core/time/time.hpp>

namespace sinen {
float Time::Seconds() { return static_cast<float>(SDL_GetTicks() / 1000.f); }

uint32_t Time::Milli() { return SDL_GetTicks(); }
float Time::DeltaTime() { return SceneSystem::delta_time(); }
} // namespace sinen
