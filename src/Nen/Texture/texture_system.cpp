#include "texture_system.hpp"
#include <Nen.hpp>

namespace nen {
void SDLObjectCloser::operator()(::SDL_Surface *surface) {
  if (surface != nullptr) {
    ::SDL_FreeSurface(surface);
    surface = nullptr;
  }
}
void SDLObjectCloser::operator()(::SDL_RWops *rw) {
  if (rw != nullptr) {
    ::SDL_FreeRW(rw);
    rw = nullptr;
  }
}
handle_t texture_system::create() {
  auto *surf = SDL_CreateRGBSurface(0, 1, 1, 32, 0, 0, 0, 0);
  handle_t handle =
      surfaces.move(std::unique_ptr<SDL_Surface, SDLObjectCloser>(surf));
  return handle;
}
bool texture_system::Contain(handle_t handle) {
  return surfaces.contains(handle);
}
void texture_system::remove(handle_t handle) { surfaces.remove(handle); }
SDL_Surface &texture_system::get(handle_t handle) {
  return surfaces.get(handle);
}

} // namespace nen