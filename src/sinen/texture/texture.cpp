#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <any>
#include <cassert>
#include <d3d10.h>
#include <io/data_stream.hpp>
#include <logger/logger.hpp>
#include <memory>
#include <optional>
#include <string_view>
#include <texture/texture.hpp>
#include <unordered_map>
#include <utility/handle_t.hpp>

#include "../render/render_system.hpp"
#include "texture_data.hpp"

#include <paranoixa/paranoixa.hpp>

namespace sinen {

struct SDLObjectCloser {
  void operator()(::SDL_Surface *surface);
  void operator()(::SDL_IOStream *rw);
};
void SDLObjectCloser::operator()(::SDL_Surface *surface) {
  if (surface) {
    SDL_DestroySurface(surface);
  }
}
void SDLObjectCloser::operator()(::SDL_IOStream *rw) { SDL_CloseIO(rw); }
SDL_Surface &get(HandleT handle) {
  SDL_Surface *surf = reinterpret_cast<SDL_Surface *>(handle);
  return *surf;
}
Texture::Texture() {
  auto data = std::make_shared<TextureData>();
  data->handle = create();
  data->texture = nullptr;
  this->textureData = data;
}
Texture::~Texture() {}

bool Texture::load(std::string_view fileName) {
  auto texdata = GetTexData(textureData);
  auto &surface = get(texdata->handle);
  auto *src_surface = ::IMG_Load_IO(
      (SDL_IOStream *)DataStream::open_as_rwops(AssetType::Texture, fileName),
      0);
  if (!src_surface) {
    Logger::error("Texture load failed.");
    return false;
  }
  memcpy(&surface, src_surface, sizeof(SDL_Surface));

  texdata->texture = CreateNativeTexture(texdata->handle);
  return true;
}
bool Texture::load_from_memory(std::vector<char> &buffer) {
  auto texdata = GetTexData(textureData);
  auto &surface = get(texdata->handle);
  auto rw = std::unique_ptr<::SDL_IOStream, SDLObjectCloser>(
      ::SDL_IOFromMem(reinterpret_cast<void *>(buffer.data()), buffer.size()));
  if (!rw) {
    return false;
  }
  auto *src_surface = ::IMG_Load_IO(rw.get(), 0);
  if (!src_surface) {
    return false;
  }
  memcpy(&surface, src_surface, sizeof(SDL_Surface));
  texdata->texture = CreateNativeTexture(texdata->handle);
  return true;
}

void Texture::fill_color(const Color &color) {
  auto texdata = GetTexData(textureData);
  auto &surface = get(texdata->handle);
  ::SDL_FillSurfaceRect(
      &surface, NULL,
      ::SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32), nullptr,
                    color.r * 255, color.g * 255, color.b * 255,
                    color.a * 255));
  if (texdata->texture) {
    UpdateNativeTexture(texdata->texture, texdata->handle);
  } else {
    texdata->texture = CreateNativeTexture(texdata->handle);
  }
}
void Texture::blend_color(const Color &color) {
  auto texdata = GetTexData(textureData);
  auto &surface = get(texdata->handle);
  SDL_SetSurfaceBlendMode(&surface, SDL_BLENDMODE_BLEND);
  SDL_SetSurfaceColorMod(&surface, color.r * 255, color.g * 255, color.b * 255);
  SDL_SetSurfaceAlphaMod(&surface, color.a * 255);
  SDL_BlitSurface(&surface, NULL, (SDL_Surface *)texdata->handle, NULL);
  if (texdata->texture) {
    UpdateNativeTexture(texdata->texture, texdata->handle);
  } else {
    texdata->texture = CreateNativeTexture(texdata->handle);
  }
}

Texture Texture::copy() {
  auto texdata = GetTexData(textureData);
  auto &src = get(texdata->handle);
  Texture dst_texture;
  auto &dst = get(GetTexData(dst_texture.textureData)->handle);
  dst.w = src.w;
  dst.h = src.h;
  SDL_BlitSurface(&src, NULL, &dst, NULL);
  GetTexData(dst_texture.textureData)->texture = texdata->texture;
  return dst_texture;
}

Vector2 Texture::size() {
  auto &surface = get(GetTexData(textureData)->handle);
  return Vector2(static_cast<float>(surface.w), static_cast<float>(surface.h));
}

HandleT Texture::create() {
  auto *surf = SDL_CreateSurface(1, 1, SDL_PIXELFORMAT_RGBA8888);
  HandleT handle = reinterpret_cast<HandleT>(surf);
  return handle;
}

} // namespace sinen
