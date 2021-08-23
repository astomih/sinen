#include <Time/Time.hpp>
#include <SDL.h>

namespace nen
{
    float Time::GetTicksAsSeconds()
    {
        return static_cast<float>(SDL_GetTicks() / 1000.f);
    }

    uint32_t Time::GetTicksAsMilliSeconds()
    {
        return SDL_GetTicks();
    }
}