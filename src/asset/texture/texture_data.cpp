#include "texture_data.hpp"
#include "../../graphics/graphics_system.hpp"
#include <core/logger/logger.hpp>

#include "SDL3/SDL_pixels.h"
#include <SDL3/SDL.h>
namespace sinen {
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