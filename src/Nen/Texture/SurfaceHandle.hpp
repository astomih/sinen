#pragma once
#include <SDL.h>
#include <memory>
#include <string_view>
#include <unordered_map>

namespace nen {
struct SDLObjectCloser {
  void operator()(::SDL_Surface *surface);
  void operator()(::SDL_RWops *rw);
};
class surface_handler {
public:
  static void Register(std::string_view string,
                       std::unique_ptr<::SDL_Surface, SDLObjectCloser> surface);
  static bool Contain(std::string_view string);
  static SDL_Surface &Load(std::string_view string);

private:
  static std::unordered_map<std::string,
                            std::unique_ptr<::SDL_Surface, SDLObjectCloser>>
      surfaces;
};
} // namespace nen
