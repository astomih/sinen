#include "webgpu_copy_pass.hpp"

#ifndef EMSCRIPTEN
#include "webgpu_buffer.hpp"
#include "webgpu_command_buffer.hpp"
#include "webgpu_convert.hpp"
#include "webgpu_device.hpp"
#include "webgpu_texture.hpp"
#include "webgpu_transfer_buffer.hpp"

#include <SDL3/SDL.h>

namespace sinen::gpu::webgpu {
void CopyPass::uploadTexture(const TextureTransferInfo &src,
                             const TextureRegion &dst, bool cycle) {
  (void)cycle;
  auto transfer = downCast<TransferBuffer>(src.transferBuffer);
  auto texture = downCast<Texture>(dst.texture);
  if (!transfer || !texture) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "uploadTexture failed: invalid transfer buffer or texture");
    return;
  }

  const UInt8 *uploadData = transfer->getUploadData();
  if (!uploadData) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "uploadTexture failed: upload buffer is empty");
    return;
  }

  const auto textureFormat = texture->getCreateInfo().format;
  const UInt32 bpp = convert::bytesPerPixel(textureFormat);
  const UInt32 bytesPerRow = dst.width * bpp;

  WGPUTexelCopyTextureInfo destination{};
  destination.texture = texture->getNative();
  destination.mipLevel = dst.mipLevel;
  destination.origin = {dst.x, dst.y, dst.z};
  destination.aspect = WGPUTextureAspect_All;

  WGPUTexelCopyBufferLayout layout{};
  layout.offset = src.offset;
  layout.bytesPerRow = bytesPerRow;
  layout.rowsPerImage = dst.height;

  WGPUExtent3D writeSize{};
  writeSize.width = dst.width;
  writeSize.height = dst.height;
  writeSize.depthOrArrayLayers = dst.depth;

  size_t uploadSize =
      static_cast<size_t>(bytesPerRow) * dst.height * dst.depth;
  if (src.offset + uploadSize > transfer->getCreateInfo().size) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "uploadTexture failed: source range exceeds transfer buffer");
    return;
  }
  wgpuQueueWriteTexture(commandBuffer.getDevice()->getQueue(), &destination,
                        uploadData + src.offset, uploadSize, &layout, &writeSize);
}

void CopyPass::downloadTexture(const TextureRegion &src,
                               const TextureTransferInfo &dst) {
  auto transfer = downCast<TransferBuffer>(dst.transferBuffer);
  auto texture = downCast<Texture>(src.texture);
  if (!transfer || !texture || !transfer->getNative()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "downloadTexture failed: invalid transfer buffer or texture");
    return;
  }

  const auto textureFormat = texture->getCreateInfo().format;
  const UInt32 bpp = convert::bytesPerPixel(textureFormat);
  const UInt32 bytesPerRow = src.width * bpp;

  WGPUTexelCopyTextureInfo source{};
  source.texture = texture->getNative();
  source.mipLevel = src.mipLevel;
  source.origin = {src.x, src.y, src.z};
  source.aspect = WGPUTextureAspect_All;

  WGPUTexelCopyBufferInfo destination{};
  destination.buffer = transfer->getNative();
  destination.layout.offset = dst.offset;
  destination.layout.bytesPerRow = bytesPerRow;
  destination.layout.rowsPerImage = src.height;

  WGPUExtent3D copySize{};
  copySize.width = src.width;
  copySize.height = src.height;
  copySize.depthOrArrayLayers = src.depth;

  wgpuCommandEncoderCopyTextureToBuffer(commandBuffer.getEncoder(), &source,
                                        &destination, &copySize);
}

void CopyPass::uploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool cycle) {
  (void)cycle;
  auto transfer = downCast<TransferBuffer>(src.transferBuffer);
  auto buffer = downCast<Buffer>(dst.buffer);
  if (!transfer || !buffer) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "uploadBuffer failed: invalid transfer buffer or destination");
    return;
  }
  const UInt8 *uploadData = transfer->getUploadData();
  if (!uploadData) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "uploadBuffer failed: upload buffer is empty");
    return;
  }
  if (src.offset + dst.size > transfer->getCreateInfo().size) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "uploadBuffer failed: source range exceeds transfer buffer");
    return;
  }
  wgpuQueueWriteBuffer(commandBuffer.getDevice()->getQueue(), buffer->getNative(),
                       dst.offset, uploadData + src.offset, dst.size);
}

void CopyPass::downloadBuffer(const BufferRegion &src,
                              const BufferTransferInfo &dst) {
  auto transfer = downCast<TransferBuffer>(dst.transferBuffer);
  auto buffer = downCast<Buffer>(src.buffer);
  if (!transfer || !buffer || !transfer->getNative()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "downloadBuffer failed: invalid source or transfer buffer");
    return;
  }
  wgpuCommandEncoderCopyBufferToBuffer(commandBuffer.getEncoder(), buffer->getNative(),
                                       src.offset, transfer->getNative(),
                                       dst.offset, src.size);
}

void CopyPass::copyTexture(const TextureLocation &src, const TextureLocation &dst,
                           UInt32 width, UInt32 height, UInt32 depth,
                           bool cycle) {
  (void)cycle;
  auto srcTex = downCast<Texture>(src.texture);
  auto dstTex = downCast<Texture>(dst.texture);
  if (!srcTex || !dstTex) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "copyTexture failed: invalid source or destination texture");
    return;
  }

  WGPUTexelCopyTextureInfo source{};
  source.texture = srcTex->getNative();
  source.mipLevel = src.mipLevel;
  source.origin = {src.x, src.y, src.z};
  source.aspect = WGPUTextureAspect_All;

  WGPUTexelCopyTextureInfo destination{};
  destination.texture = dstTex->getNative();
  destination.mipLevel = dst.mipLevel;
  destination.origin = {dst.x, dst.y, dst.z};
  destination.aspect = WGPUTextureAspect_All;

  WGPUExtent3D copySize{};
  copySize.width = width;
  copySize.height = height;
  copySize.depthOrArrayLayers = depth;

  wgpuCommandEncoderCopyTextureToTexture(commandBuffer.getEncoder(), &source,
                                         &destination, &copySize);
}
} // namespace sinen::gpu::webgpu

#endif // EMSCRIPTEN
