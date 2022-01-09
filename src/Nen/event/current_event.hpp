#pragma once
#include "SDL_events.h"
#include "Utility/Singleton.hpp"
#include "imgui_impl_sdl.h"
#include <Nen.hpp>
#include <SDL.h>

namespace nen {
class current_event_handle {

public:
  static SDL_Event current_event;
};
} // namespace nen