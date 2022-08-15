#pragma once
#include <SDL.h>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <utility/handler.hpp>

namespace sinen {
struct SDLObjectCloser {
  void operator()(::SDL_Surface *surface);
  void operator()(::SDL_RWops *rw);
};
class texture_system {
public:
  texture_system() = default;
  handle_t create();
  bool contains(handle_t);
  SDL_Surface &get(handle_t);
  SDL_Surface *get_raw(handle_t);
  void move(handle_t, std::unique_ptr<SDL_Surface, SDLObjectCloser>);
  void remove(handle_t);

private:
  handler<::SDL_Surface, SDLObjectCloser> m_surfaces;
};
} // namespace sinen
