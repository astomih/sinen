#include "../manager/get_system.hpp"
#include "texture_system.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <io/dstream.hpp>
#include <logger/logger.hpp>
#include <texture/texture.hpp>

namespace nen {

texture::texture() { handle = get_texture().create(); }
texture::~texture() {}

bool texture::load(std::string_view fileName) {
  auto &surface = get_texture().get(handle);
  auto *src_surface = ::IMG_Load_RW(
      (SDL_RWops *)dstream::open_as_rwops(asset_type::Texture, fileName), 0);
  if (!src_surface) {
    logger::error("%s", IMG_GetError());
    return false;
  }
  memcpy(&surface, src_surface, sizeof(SDL_Surface));
  return true;
}
bool texture::LoadFromMemory(std::vector<char> &buffer, std::string_view ID) {
  auto &surface = get_texture().get(handle);
  auto rw = std::unique_ptr<::SDL_RWops, SDLObjectCloser>(
      ::SDL_RWFromMem(reinterpret_cast<void *>(buffer.data()), buffer.size()));
  if (!rw) {
    logger::error("%s", IMG_GetError());

    return false;
  }
  auto *src_surface = ::IMG_Load_RW(rw.get(), 0);
  if (!src_surface) {
    logger::error("%s", IMG_GetError());
    return false;
  }
  memcpy(&surface, src_surface, sizeof(SDL_Surface));

  return true;
}

void texture::fill_color(const color &color) {
  auto &surface = get_texture().get(handle);
  ::SDL_FillRect(&surface, NULL,
                 ::SDL_MapRGBA(surface.format, color.r * 255, color.g * 255,
                               color.b * 255, color.a * 255));
}

vector2 texture::size() {
  auto &surface = get_texture().get(handle);
  return vector2(surface.w, surface.h);
}

} // namespace nen
