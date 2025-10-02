#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <asset/texture/texture.hpp>
#include <cassert>
#include <core/io/asset_io.hpp>
#include <core/logger/logger.hpp>
#include <cstddef>
#include <memory>
#include <string_view>

#include "texture_data.hpp"

#include <paranoixa/paranoixa.hpp>

namespace sinen {
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
  auto rawData = getTextureRawData(textureData);
  auto *pSurface = ::IMG_Load_IO(
      (SDL_IOStream *)AssetIO::openAsIOStream(AssetType::Texture, fileName), 0);
  if (!pSurface) {
    Logger::error("Texture load failed.");
    return false;
  }
  rawData->pSurface = pSurface;

  rawData->texture = CreateNativeTexture(rawData->pSurface);
  return true;
}
bool Texture::loadFromMemory(std::vector<char> &buffer) const {
  auto rawData = getTextureRawData(textureData);
  auto *io =
      ::SDL_IOFromMem(reinterpret_cast<void *>(buffer.data()), buffer.size());
  if (!io) {
    return false;
  }
  auto *src_surface = ::IMG_Load_IO(io, true);
  if (!src_surface) {
    return false;
  }
  memcpy(rawData->pSurface, src_surface, sizeof(SDL_Surface));
  rawData->texture = CreateNativeTexture(rawData->pSurface);
  return true;
}

bool Texture::loadFromMemory(void *pPixels, uint32_t width, uint32_t height) {
  auto texdata = getTextureRawData(textureData);
  texdata->pSurface = ::SDL_CreateSurfaceFrom(
      width, height, SDL_PIXELFORMAT_RGBA8888, pPixels, width * 4);
  texdata->texture = CreateNativeTexture(pPixels, width, height);
  return true;
}

void Texture::fillColor(const Color &color) {
  auto texdata = getTextureRawData(textureData);
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
void Texture::blendColor(const Color &color) {
  auto texdata = getTextureRawData(textureData);
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
  auto texdata = getTextureRawData(textureData);
  Texture dst_texture;
  auto dTexData = getTextureRawData(dst_texture.textureData);
  dTexData->pSurface = SDL_CreateSurface(
      texdata->pSurface->w, texdata->pSurface->h, texdata->pSurface->format);
  SDL_BlitSurface(texdata->pSurface, nullptr, dTexData->pSurface, nullptr);
  getTextureRawData(dst_texture.textureData)->texture = texdata->texture;
  return dst_texture;
}

glm::vec2 Texture::size() {
  auto *surface = getTextureRawData(textureData)->pSurface;
  return glm::vec2(static_cast<float>(surface->w),
                   static_cast<float>(surface->h));
}

} // namespace sinen
