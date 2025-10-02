#include "texture_data.hpp"
#include "../../graphics/graphics_system.hpp"
#include <core/logger/logger.hpp>

#include "SDL3/SDL_pixels.h"
#include <SDL3/SDL.h>
namespace sinen {
TextureData::~TextureData() { SDL_DestroySurface(pSurface); }
template <typename T> using Ptr = px::Ptr<T>;
static void writeTexture(px::Ptr<px::Texture> texture, void *pPixels) {
  auto allocator = GraphicsSystem::getAllocator();
  auto device = GraphicsSystem::getDevice();
  uint32_t width = texture->getCreateInfo().width,
           height = texture->getCreateInfo().height;
  Ptr<px::TransferBuffer> transferBuffer;
  {
    px::TransferBuffer::CreateInfo info{};
    info.allocator = allocator;
    info.size = width * height * 4;
    info.usage = px::TransferBufferUsage::Upload;
    transferBuffer = device->CreateTransferBuffer(info);
    auto *pMapped = transferBuffer->Map(true);
    memcpy(pMapped, pPixels, info.size);
    transferBuffer->Unmap();
  }
  {
    px::CommandBuffer::CreateInfo info{};
    info.allocator = allocator;
    auto commandBuffer = device->AcquireCommandBuffer(info);
    auto copyPass = commandBuffer->BeginCopyPass();
    px::TextureTransferInfo src{};
    src.offset = 0;
    src.transferBuffer = transferBuffer;
    px::TextureRegion dst{};
    dst.x = 0;
    dst.y = 0;
    dst.width = width;
    dst.height = height;
    dst.depth = 1;
    dst.texture = texture;
    copyPass->UploadTexture(src, dst, true);
    commandBuffer->EndCopyPass(copyPass);
    device->SubmitCommandBuffer(commandBuffer);
  }
  device->WaitForGPUIdle();
}
px::Ptr<px::Texture> CreateNativeTexture(void *pPixels, uint32_t width,
                                         uint32_t height) {
  auto allocator = GraphicsSystem::getAllocator();
  auto device = GraphicsSystem::getDevice();

  Ptr<px::Texture> texture;
  {
    px::Texture::CreateInfo info{};
    info.allocator = allocator;
    info.width = width;
    info.height = height;
    info.layerCountOrDepth = 1;
    info.format = px::TextureFormat::R8G8B8A8_UNORM;
    info.usage = px::TextureUsage::Sampler;
    info.numLevels = 1;
    info.sampleCount = px::SampleCount::x1;
    info.type = px::TextureType::Texture2D;
    texture = device->CreateTexture(info);
  }
  writeTexture(texture, pPixels);
  return texture;
}
Ptr<px::Texture> CreateNativeTexture(SDL_Surface *pSurface) {
  auto allocator = GraphicsSystem::getAllocator();
  auto device = GraphicsSystem::getDevice();
  auto *pImageDataSurface = ::SDL_ConvertSurface(
      pSurface, pSurface->format == SDL_PIXELFORMAT_RGBA8888
                    ? pSurface->format
                    : SDL_PIXELFORMAT_RGBA32);
  int width = pImageDataSurface->w, height = pImageDataSurface->h;
  auto texture = CreateNativeTexture(pImageDataSurface->pixels, width, height);
  SDL_DestroySurface(pImageDataSurface);
  return texture;
}
void UpdateNativeTexture(px::Ptr<px::Texture> texture, void *pPixels) {
  writeTexture(texture, pPixels);
}
void UpdateNativeTexture(Ptr<px::Texture> texture, SDL_Surface *pSurface) {
  auto *pImageDataSurface = ::SDL_ConvertSurface(
      pSurface, pSurface->format == SDL_PIXELFORMAT_RGBA8888
                    ? pSurface->format
                    : SDL_PIXELFORMAT_RGBA32);
  UpdateNativeTexture(texture, pImageDataSurface->pixels);
  SDL_DestroySurface(pImageDataSurface);
}
} // namespace sinen