#pragma once
#include <Nen.hpp>
#include <SDL.h>
#include <memory>
#include <string_view>
#include <unordered_map>

namespace nen {
struct SDLObjectCloser {
  void operator()(::SDL_Surface *surface);
  void operator()(::SDL_RWops *rw);
};
class texture_system {
public:
  texture_system(manager &_manager) : manager(_manager) {}
  handle_t create();
  bool Contain(handle_t);
  SDL_Surface &get(handle_t);
  void remove(handle_t);

private:
  manager &manager;
  handler<::SDL_Surface, SDLObjectCloser> surfaces;
};
} // namespace nen
