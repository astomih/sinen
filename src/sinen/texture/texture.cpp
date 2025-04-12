#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <cassert>
#include <cstddef>
#include <io/data_stream.hpp>
#include <logger/logger.hpp>
#include <memory>
#include <string_view>
#include <texture/texture.hpp>

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
SDL_Surface *create() {
  auto *surf = SDL_CreateSurface(1, 1, SDL_PIXELFORMAT_RGBA8888);
  return surf;
}
Texture::Texture() {
  auto data = std::make_shared<TextureData>();
  data->pSurface = create();
  data->texture = nullptr;
  this->textureData = data;
}
Texture::~Texture() {}

bool Texture::load(std::string_view fileName) {
  auto texdata = GetTexData(textureData);
  auto *pSurface = ::IMG_Load_IO(
      (SDL_IOStream *)DataStream::open_as_rwops(AssetType::Texture, fileName),
      0);
  if (!pSurface) {
    Logger::error("Texture load failed.");
    return false;
  }
  texdata->pSurface = pSurface;

  texdata->texture = CreateNativeTexture(texdata->pSurface);
  return true;
}
bool Texture::load_from_memory(std::vector<char> &buffer) {
  auto texdata = GetTexData(textureData);
  auto rw = std::unique_ptr<::SDL_IOStream, SDLObjectCloser>(
      ::SDL_IOFromMem(reinterpret_cast<void *>(buffer.data()), buffer.size()));
  if (!rw) {
    return false;
  }
  auto *src_surface = ::IMG_Load_IO(rw.get(), 0);
  if (!src_surface) {
    return false;
  }
  memcpy(texdata->pSurface, src_surface, sizeof(SDL_Surface));
  texdata->texture = CreateNativeTexture(texdata->pSurface);
  return true;
}

void Texture::fill_color(const Color &color) {
  auto texdata = GetTexData(textureData);
  ::SDL_FillSurfaceRect(
      texdata->pSurface, NULL,
      ::SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32), nullptr,
                    color.r * 255, color.g * 255, color.b * 255,
                    color.a * 255));
  if (texdata->texture) {
    UpdateNativeTexture(texdata->texture, texdata->pSurface);
  } else {
    texdata->texture = CreateNativeTexture(texdata->pSurface);
  }
}
void Texture::blend_color(const Color &color) {
  auto texdata = GetTexData(textureData);
  SDL_SetSurfaceBlendMode(texdata->pSurface, SDL_BLENDMODE_BLEND);
  SDL_SetSurfaceColorMod(texdata->pSurface, color.r * 255, color.g * 255,
                         color.b * 255);
  SDL_SetSurfaceAlphaMod(texdata->pSurface, color.a * 255);
  SDL_BlitSurface(texdata->pSurface, NULL, texdata->pSurface, NULL);
  if (texdata->texture) {
    UpdateNativeTexture(texdata->texture, texdata->pSurface);
  } else {
    texdata->texture = CreateNativeTexture(texdata->pSurface);
  }
}

Texture Texture::copy() {
  auto texdata = GetTexData(textureData);
  Texture dst_texture;
  auto dTexData = GetTexData(dst_texture.textureData);
  dTexData->pSurface = SDL_CreateSurface(
      texdata->pSurface->w, texdata->pSurface->h, texdata->pSurface->format);
  SDL_BlitSurface(texdata->pSurface, nullptr, dTexData->pSurface, nullptr);
  GetTexData(dst_texture.textureData)->texture = texdata->texture;
  return dst_texture;
}

glm::vec2 Texture::size() {
  auto *surface = GetTexData(textureData)->pSurface;
  return glm::vec2(static_cast<float>(surface->w),
                   static_cast<float>(surface->h));
}

} // namespace sinen
