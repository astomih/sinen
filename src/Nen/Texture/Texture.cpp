#include "texture_system.hpp"
#include <IO/AssetReader.hpp>
#include <Logger/Logger.hpp>
#include <SDL.h>
#include <SDL_image.h>
#include <Texture/Texture.hpp>
#include <manager/manager.hpp>

namespace nen {

texture::texture() { handle = get_texture_system().create(); }
texture::~texture() {}

bool texture::Load(std::string_view fileName) {
  auto &surface = get_texture_system().get(handle);
  auto *src_surface = ::IMG_Load_RW(
      (SDL_RWops *)data_io::LoadAsRWops(asset_type::Texture, fileName), 0);
  if (!src_surface) {
    logger::Error("%s", IMG_GetError());
    return false;
  }
  memcpy(&surface, src_surface, sizeof(SDL_Surface));
  return true;
}
bool texture::LoadFromMemory(std::vector<char> &buffer, std::string_view ID) {
  auto &surface = get_texture_system().get(handle);
  auto rw = std::unique_ptr<::SDL_RWops, SDLObjectCloser>(
      ::SDL_RWFromMem(reinterpret_cast<void *>(buffer.data()), buffer.size()));
  if (!rw) {
    logger::Error("%s", IMG_GetError());

    return false;
  }
  auto *src_surface = ::IMG_Load_RW(rw.get(), 0);
  if (!src_surface) {
    logger::Error("%s", IMG_GetError());
    return false;
  }
  memcpy(&surface, src_surface, sizeof(SDL_Surface));

  return true;
}

void texture::fill_color(const color &color) {
  auto &surface = get_texture_system().get(handle);
  ::SDL_FillRect(&surface, NULL,
                 ::SDL_MapRGBA(surface.format, color.r * 255, color.g * 255,
                               color.b * 255, color.a * 255));
}

vector2 texture::size() {
  auto &surface = get_texture_system().get(handle);
  return vector2(surface.w, surface.h);
}

} // namespace nen
