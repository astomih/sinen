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
Texture::Texture() {
  auto data = std::make_shared<TextureData>();
  data->pSurface = SDL_CreateSurface(1, 1, SDL_PIXELFORMAT_RGBA32);
  data->texture = nullptr;
  this->textureData = data;
}
Texture::Texture(int width, int height) {
  auto data = std::make_shared<TextureData>();
  data->pSurface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
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

  rawData->texture = createNativeTexture(rawData->pSurface);
  return true;
}
bool Texture::loadFromPath(std::string_view path) {
  auto rawData = getTextureRawData(textureData);
  auto *pSurface = ::IMG_Load(path.data());
  if (!pSurface) {
    Logger::error("Texture load failed.");
    return false;
  }
  rawData->pSurface = pSurface;

  rawData->texture = createNativeTexture(rawData->pSurface);
  return true;
}

bool Texture::loadFromMemory(std::vector<char> &buffer) const {
  auto rawData = getTextureRawData(textureData);
  auto *io = ::SDL_IOFromConstMem(buffer.data(), buffer.size());
  if (!io) {
    return false;
  }
  auto *pSrcSurface = ::IMG_Load_IO(io, true);
  if (!pSrcSurface) {
    return false;
  }
  rawData->pSurface = ::SDL_CreateSurface(pSrcSurface->w, pSrcSurface->h,
                                          SDL_PIXELFORMAT_RGBA32);
  ::SDL_BlitSurface(pSrcSurface, nullptr, rawData->pSurface, nullptr);
  rawData->texture = createNativeTexture(rawData->pSurface);
  SDL_DestroySurface(pSrcSurface);
  return true;
}

bool Texture::loadFromMemory(void *pPixels, uint32_t width, uint32_t height) {
  auto texdata = getTextureRawData(textureData);
  texdata->pSurface = ::SDL_CreateSurfaceFrom(
      width, height, SDL_PIXELFORMAT_RGBA32, pPixels, width * 4);
  texdata->texture = createNativeTexture(
      pPixels, px::TextureFormat::R8G8B8A8_UNORM, width, height);
  return true;
}

void Texture::fill(const Color &color) {
  auto data = getTextureRawData(textureData);
  ::SDL_FillSurfaceRect(
      data->pSurface, nullptr,
      ::SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32), nullptr,
                    color.r * 255, color.g * 255, color.b * 255,
                    color.a * 255));
  if (data->texture) {
    UpdateNativeTexture(data->texture, data->pSurface);
  } else {
    data->texture = createNativeTexture(data->pSurface);
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
