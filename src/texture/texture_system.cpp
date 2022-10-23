#include "texture_system.hpp"

namespace sinen {
void SDLObjectCloser::operator()(::SDL_Surface *surface) {
  ::SDL_FreeSurface(surface);
}
void SDLObjectCloser::operator()(::SDL_RWops *rw) { ::SDL_FreeRW(rw); }
handle_t texture_system::create() {
  auto *surf =
      SDL_CreateRGBSurfaceWithFormat(0, 1, 1, 32, SDL_PIXELFORMAT_RGBA32);
  auto ptr = std::unique_ptr<SDL_Surface, SDLObjectCloser>(surf);
  return m_surfaces.move(std::move(ptr));
}
bool texture_system::contains(handle_t handle) {
  return m_surfaces.contains(handle);
}
void texture_system::remove(handle_t handle) { m_surfaces.remove(handle); }
SDL_Surface &texture_system::get(handle_t handle) {
  return m_surfaces.get(handle);
}
SDL_Surface *texture_system::get_raw(handle_t handle) {
  return m_surfaces.get_raw(handle);
}
void texture_system::move(handle_t handle,
                          std::unique_ptr<SDL_Surface, SDLObjectCloser> ptr) {
  return m_surfaces.move(handle, std::move(ptr));
}
void texture_system::shutdown() { m_surfaces.clear(); }

} // namespace sinen
