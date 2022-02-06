#pragma once
#include <SDL.h>

namespace nen {
class current_event_handle {

public:
  static SDL_Event current_event;
};
} // namespace nen