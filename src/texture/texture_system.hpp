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
  static handle_t create();
  static bool contains(handle_t);
  static SDL_Surface &get(handle_t);
  static SDL_Surface *get_raw(handle_t);
  static void move(handle_t, std::unique_ptr<SDL_Surface, SDLObjectCloser>);
  static void remove(handle_t);
  static void shutdown();

private:
  static handler<::SDL_Surface, SDLObjectCloser> m_surfaces;
};
} // namespace sinen
