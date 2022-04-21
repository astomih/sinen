#include "texture_system.hpp"

namespace nen {
void SDLObjectCloser::operator()(::SDL_Surface *surface) {
  ::SDL_FreeSurface(surface);
}
void SDLObjectCloser::operator()(::SDL_RWops *rw) { ::SDL_FreeRW(rw); }
handle_t texture_system::create() {
  auto *surf =
      SDL_CreateRGBSurfaceWithFormat(0, 1, 1, 32, SDL_PIXELFORMAT_RGBA32);
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
SDL_Surface *texture_system::get_raw(handle_t handle) {
  return surfaces.get_raw(handle);
}
void texture_system::move(handle_t handle,
                          std::unique_ptr<SDL_Surface, SDLObjectCloser> ptr) {
  return surfaces.move(handle, std::move(ptr));
}

} // namespace nen