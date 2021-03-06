#include "../manager/get_system.hpp"
#include "texture_system.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <io/dstream.hpp>
#include <logger/logger.hpp>
#include <texture/texture.hpp>

namespace nen {

texture::texture() {
  handle = get_texture().create();
  is_need_update = std::make_shared<bool>(false);
}
texture::~texture() {}

bool texture::load(std::string_view fileName) {
  *is_need_update = true;
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
bool texture::load_from_memory(std::vector<char> &buffer, std::string_view ID) {
  *is_need_update = true;
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
  *is_need_update = true;
  auto &surface = get_texture().get(handle);
  ::SDL_FillRect(&surface, NULL,
                 ::SDL_MapRGBA(surface.format, color.r * 255, color.g * 255,
                               color.b * 255, color.a * 255));
}
void texture::blend_color(const color &color) {
  *is_need_update = true;
  auto &surface = get_texture().get(handle);
  SDL_SetSurfaceBlendMode(&surface, SDL_BLENDMODE_BLEND);
  SDL_SetSurfaceColorMod(&surface, color.r * 255, color.g * 255, color.b * 255);
  SDL_SetSurfaceAlphaMod(&surface, color.a * 255);
  auto *tmp = SDL_CreateRGBSurface(0, surface.w, surface.h, 32, 0, 0, 0, 0);
  SDL_BlitSurface(&surface, NULL, tmp, NULL);
  get_texture().move(handle,
                     std::unique_ptr<SDL_Surface, SDLObjectCloser>(tmp));
}

texture texture::copy() {
  auto &src = get_texture().get(handle);
  texture dst_texture;
  auto &dst = get_texture().get(dst_texture.handle);
  dst.w = src.w;
  dst.h = src.h;
  SDL_BlitSurface(&src, NULL, &dst, NULL);
  *dst_texture.is_need_update = true;
  return dst_texture;
}

vector2 texture::size() {
  auto &surface = get_texture().get(handle);
  return vector2(surface.w, surface.h);
}

} // namespace nen
