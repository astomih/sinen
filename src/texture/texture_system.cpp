#include "texture_system.hpp"

namespace sinen {
void SDLObjectCloser::operator()(::SDL_Surface *surface) {
  if (surface) {
    SDL_FreeSurface(surface);
  }
}
void SDLObjectCloser::operator()(::SDL_RWops *rw) { ::SDL_FreeRW(rw); }
handle_t texture_system::create() {
  auto *surf =
      SDL_CreateRGBSurfaceWithFormat(0, 1, 1, 32, SDL_PIXELFORMAT_RGBA32);
  handle_t handle = reinterpret_cast<handle_t>(surf);
  return handle;
}
bool texture_system::contains(handle_t handle) {
  return handle == 0 ? false : true;
}
void texture_system::remove(handle_t handle) {
  if (!handle) {
    return;
  }
  SDL_Surface *surf = reinterpret_cast<SDL_Surface *>(handle);
  SDL_FreeSurface(surf);
  surf = nullptr;
  handle = NULL;
}
SDL_Surface &texture_system::get(handle_t handle) {
  SDL_Surface *surf = reinterpret_cast<SDL_Surface *>(handle);
  return *surf;
}
SDL_Surface *texture_system::get_raw(handle_t handle) {
  SDL_Surface *surf = reinterpret_cast<SDL_Surface *>(handle);
  return surf;
}
void texture_system::shutdown() {}

} // namespace sinen
