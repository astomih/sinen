#include <SDL3/SDL.h>
#include <asset/texture/texture.hpp>
#include <cassert>
#include <core/io/asset_io.hpp>
#include <core/logger/logger.hpp>
#include <graphics/graphics_system.hpp>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <graphics/rhi/rhi.hpp>

namespace sinen {
static Ptr<rhi::Texture> createNativeTexture(void *pPixels,
                                             rhi::TextureFormat textureFormat,
                                             uint32_t width, uint32_t height);
static void updateNativeTexture(Ptr<rhi::Texture> texture, void *pPixels);
Texture::Texture() { this->texture = nullptr; }
Texture::Texture(int width, int height) {
  Array<uint8_t> pixels(width * height * 4, 0);
  this->texture = createNativeTexture(
      pixels.data(), rhi::TextureFormat::R8G8B8A8_UNORM, width, height);
}
Texture::~Texture() {}

bool Texture::load(StringView fileName) {
  unsigned char *pixels;
  int width;
  int height;
  int bpp;

  auto str = AssetIO::openAsString(fileName);

  pixels = stbi_load_from_memory(reinterpret_cast<unsigned char *>(str.data()),
                                 str.size(), &width, &height, &bpp, 4);

  texture = createNativeTexture(pixels, rhi::TextureFormat::R8G8B8A8_UNORM,
                                width, height);
  return true;
}
bool Texture::load(const Buffer &buffer) {

  int width, height;
  int bpp;

  auto *pixels =
      stbi_load_from_memory(reinterpret_cast<unsigned char *>(buffer.data()),
                            buffer.size(), &width, &height, &bpp, 4);
  texture = createNativeTexture(pixels, rhi::TextureFormat::R8G8B8A8_UNORM,
                                width, height);
  return true;
}

bool Texture::loadFromMemory(Array<char> &buffer) {
  int width;
  int height;
  int bpp;

  auto *pixels =
      stbi_load_from_memory(reinterpret_cast<unsigned char *>(buffer.data()),
                            buffer.size(), &width, &height, &bpp, 4);

  this->texture = createNativeTexture(
      pixels, rhi::TextureFormat::R8G8B8A8_UNORM, width, height);
  return true;
}

bool Texture::loadFromMemory(void *pPixels, uint32_t width, uint32_t height) {
  texture = createNativeTexture(pPixels, rhi::TextureFormat::R8G8B8A8_UNORM,
                                width, height);
  return true;
}

void Texture::fill(const Color &color) {
  if (texture) {

    auto w = texture->getCreateInfo().width;
    auto h = texture->getCreateInfo().height;
    Array<uint8_t> pixels(w * h * 4, 0);
    for (int i = 0; i < w * h * 4; i += 4) {
      pixels[i + 0] = color.r * 255;
      pixels[i + 1] = color.g * 255;
      pixels[i + 2] = color.b * 255;
      pixels[i + 3] = color.a * 255;
    }
    updateNativeTexture(texture, pixels.data());
  } else {
    Array<uint8_t> pixels(4, 0);
    pixels[0] = color.r * 255;
    pixels[1] = color.g * 255;
    pixels[2] = color.b * 255;
    pixels[3] = color.a * 255;
    texture = createNativeTexture(pixels.data(),
                                  rhi::TextureFormat::R8G8B8A8_UNORM, 1, 1);
  }
}

Texture Texture::copy() {
  Texture dst;
  dst.texture = texture; // ?
  return dst;
}

glm::vec2 Texture::size() {
  auto desc = texture->getCreateInfo();
  return glm::vec2(static_cast<float>(desc.width),
                   static_cast<float>(desc.height));
}

static void writeTexture(Ptr<rhi::Texture> texture, void *pPixels) {
  auto allocator = GlobalAllocator::get();
  auto device = GraphicsSystem::getDevice();
  uint32_t width = texture->getCreateInfo().width,
           height = texture->getCreateInfo().height;
  Ptr<rhi::TransferBuffer> transferBuffer;
  {
    rhi::TransferBuffer::CreateInfo info{};
    info.allocator = allocator;
    info.size = width * height * 4;
    info.usage = rhi::TransferBufferUsage::Upload;
    transferBuffer = device->createTransferBuffer(info);
    auto *pMapped = transferBuffer->map(true);
    memcpy(pMapped, pPixels, info.size);
    transferBuffer->unmap();
  }
  {
    rhi::CommandBuffer::CreateInfo info{};
    info.allocator = allocator;
    auto commandBuffer = device->acquireCommandBuffer(info);
    auto copyPass = commandBuffer->beginCopyPass();
    rhi::TextureTransferInfo src{};
    src.offset = 0;
    src.transferBuffer = transferBuffer;
    rhi::TextureRegion dst{};
    dst.x = 0;
    dst.y = 0;
    dst.width = width;
    dst.height = height;
    dst.depth = 1;
    dst.texture = texture;
    copyPass->uploadTexture(src, dst, true);
    commandBuffer->endCopyPass(copyPass);
    device->submitCommandBuffer(commandBuffer);
  }
  device->waitForGpuIdle();
}
Ptr<rhi::Texture> createNativeTexture(void *pPixels,
                                      rhi::TextureFormat textureFormat,
                                      uint32_t width, uint32_t height) {
  auto allocator = GlobalAllocator::get();
  auto device = GraphicsSystem::getDevice();

  Ptr<rhi::Texture> texture;
  {
    rhi::Texture::CreateInfo info{};
    info.allocator = allocator;
    info.width = width;
    info.height = height;
    info.layerCountOrDepth = 1;
    info.format = textureFormat;
    info.usage = rhi::TextureUsage::Sampler;
    info.numLevels = 1;
    info.sampleCount = rhi::SampleCount::x1;
    info.type = rhi::TextureType::Texture2D;
    texture = device->createTexture(info);
  }
  writeTexture(texture, pPixels);
  return texture;
}
Ptr<rhi::Texture> createNativeTexture(SDL_Surface *pSurface) {
  auto allocator = GlobalAllocator::get();
  auto device = GraphicsSystem::getDevice();
  auto *pImageDataSurface =
      ::SDL_ConvertSurface(pSurface, SDL_PIXELFORMAT_RGBA32);
  int width = pImageDataSurface->w, height = pImageDataSurface->h;
  auto texture =
      createNativeTexture(pImageDataSurface->pixels,
                          rhi::TextureFormat::R8G8B8A8_UNORM, width, height);
  SDL_DestroySurface(pImageDataSurface);
  return texture;
}
void updateNativeTexture(Ptr<rhi::Texture> texture, void *pPixels) {
  writeTexture(texture, pPixels);
}
void updateNativeTexture(Ptr<rhi::Texture> texture, SDL_Surface *pSurface) {
  auto *pImageDataSurface = ::SDL_ConvertSurface(
      pSurface, pSurface->format == SDL_PIXELFORMAT_RGBA8888
                    ? pSurface->format
                    : SDL_PIXELFORMAT_RGBA32);
  updateNativeTexture(texture, pImageDataSurface->pixels);
  SDL_DestroySurface(pImageDataSurface);
}
} // namespace sinen
