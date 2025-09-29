#include "texture_data.hpp"
#include "../../graphics/graphics_system.hpp"
#include <core/logger/logger.hpp>

#include "SDL3/SDL_pixels.h"
#include <SDL3/SDL.h>
namespace sinen {
TextureData::~TextureData() { SDL_DestroySurface(pSurface); }
template <typename T> using Ptr = px::Ptr<T>;
Ptr<px::Texture> CreateNativeTexture(SDL_Surface *pSurface) {
  auto allocator = GraphicsSystem::GetAllocator();
  auto device = GraphicsSystem::GetDevice();
  auto *pImageDataSurface = ::SDL_ConvertSurface(pSurface, pSurface->format);

  Ptr<px::TransferBuffer> transferBuffer;
  int width = pImageDataSurface->w, height = pImageDataSurface->h;
  {
    px::TransferBuffer::CreateInfo info{};
    info.allocator = allocator;
    info.size = width * height * 4;
    info.usage = px::TransferBufferUsage::Upload;
    transferBuffer = device->CreateTransferBuffer(info);
    auto *pMapped = transferBuffer->Map(true);
    auto *pImage = pImageDataSurface->pixels;
    memcpy(pMapped, pImage, info.size);
    transferBuffer->Unmap();
  }
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
  SDL_DestroySurface(pImageDataSurface);
  return texture;
}
void UpdateNativeTexture(Ptr<px::Texture> texture, SDL_Surface *pSurface) {
  auto allocator = GraphicsSystem::GetAllocator();
  auto device = GraphicsSystem::GetDevice();
  auto *pImageDataSurface = ::SDL_ConvertSurface(pSurface, pSurface->format);

  Ptr<px::TransferBuffer> transferBuffer;
  int width = pImageDataSurface->w, height = pImageDataSurface->h;
  {
    px::TransferBuffer::CreateInfo info{};
    info.allocator = allocator;
    info.size = width * height * 4;
    info.usage = px::TransferBufferUsage::Upload;
    transferBuffer = device->CreateTransferBuffer(info);
    auto *pMapped = transferBuffer->Map(true);
    auto *pImage = pImageDataSurface->pixels;
    memcpy(pMapped, pImage, info.size);
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
  SDL_DestroySurface(pImageDataSurface);
}
} // namespace sinen