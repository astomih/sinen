#include <SDL3/SDL.h>
#include <asset/texture/texture.hpp>
#include <cassert>
#include <core/io/asset_io.hpp>
#include <core/logger/logger.hpp>
#include <cstddef>
#include <memory>
#include <string_view>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "SDL3_image/SDL_image.h"
#include "texture_data.hpp"

#include <paranoixa/paranoixa.hpp>

namespace sinen {
Texture::Texture() {
  this->texture = nullptr;
}
Texture::Texture(int width, int height) {
  std::vector<uint8_t> pixels(width*height*4,0);
  this->texture = createNativeTexture(pixels.data(),px::TextureFormat::R8G8B8A8_UNORM,width,height);
}
Texture::~Texture() {}

bool Texture::load(std::string_view fileName) {
  unsigned char* pixels;
  int width;
  int height;
  int bpp;

  auto str = AssetIO::openAsString(AssetType::Texture,fileName);

 pixels= stbi_load_from_memory(reinterpret_cast<unsigned char *>(str.data()),str.length(),&width,&height,&bpp,8);

  texture = createNativeTexture(pixels,px::TextureFormat::R8G8B8A8_UNORM,width,height);
  return true;
}
bool Texture::loadFromPath(std::string_view path) {

  int width, height;
  int bpp;

  auto* pixels = stbi_load(path.data(),&width,&height,&bpp,8);
  texture = createNativeTexture(pixels,px::TextureFormat::R8G8B8A8_UNORM,width,height);
  return true;
}

bool Texture::loadFromMemory(std::vector<char> &buffer) {
  int width;
  int height;
  int bpp;


 auto* pixels= stbi_load_from_memory(reinterpret_cast<unsigned char *>(buffer.data()),buffer.size(),&width,&height,&bpp,8);

  this->texture = createNativeTexture(pixels,px::TextureFormat::R8G8B8A8_UNORM,width,height);
  return true;
}

bool Texture::loadFromMemory(void *pPixels, uint32_t width, uint32_t height) {
  texture = createNativeTexture(
      pPixels, px::TextureFormat::R8G8B8A8_UNORM, width, height);
  return true;
}

void Texture::fill(const Color &color) {
  if (texture) {

    auto w = texture->getCreateInfo().width;
    auto h = texture->getCreateInfo().height;
  std::vector<uint8_t> pixels(w*h*4,0);
    for (int i=0;i<w*h*4;i+=4) {
       pixels[i+0] = color.r ;
      pixels[i+1] = color.g;
       pixels[i+2] = color.b;
       pixels[i+3] = color.a;
    }
    UpdateNativeTexture(texture,pixels.data());
  } else {
  std::vector<uint8_t> pixels(4,0);
    pixels[0] = color.r;
    pixels[1] = color.g;
    pixels[2] = color.b;
    pixels[3] = color.a;
    texture = createNativeTexture(pixels.data(),px::TextureFormat::R8G8B8A8_UNORM,1,1);
  }
}

Texture Texture::copy() {
  Texture dst;
  dst.texture = texture; // ?
  return dst;
}

glm::vec2 Texture::size() {
  auto desc = texture->getCreateInfo();
  return glm::vec2(static_cast<float>(desc.width),static_cast<float>(desc.height));
}

} // namespace sinen
