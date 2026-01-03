#include "sdlgpu_copy_pass.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_buffer.hpp"
#include "sdlgpu_command_buffer.hpp"
#include "sdlgpu_convert.hpp"
#include "sdlgpu_texture.hpp"
#include "sdlgpu_transfer_buffer.hpp"

#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
void CopyPass::uploadTexture(const TextureTransferInfo &src,
                             const TextureRegion &dst, bool cycle) {
  SDL_GPUTextureTransferInfo transferInfo = {
      .transfer_buffer =
          downCast<TransferBuffer>(src.transferBuffer)->getNative(),
      .offset = src.offset,
  };
  SDL_GPUTextureRegion region = {
      .texture = downCast<Texture>(dst.texture)->getNative(),
      .mip_level = dst.mipLevel,
      .layer = dst.layer,
      .x = dst.x,
      .y = dst.y,
      .z = dst.z,
      .w = dst.width,
      .h = dst.height,
      .d = dst.depth,
  };
  SDL_UploadToGPUTexture(this->copyPass, &transferInfo, &region, cycle);
}

void CopyPass::downloadTexture(const TextureRegion &src,
                               const TextureTransferInfo &dst) {
  SDL_GPUTextureTransferInfo transferInfo = {
      .transfer_buffer =
          downCast<TransferBuffer>(dst.transferBuffer)->getNative(),
      .offset = dst.offset,
  };
  SDL_GPUTextureRegion region = {
      .texture = downCast<Texture>(src.texture)->getNative(),
      .x = src.x,
      .y = src.y,
      .z = src.z,
      .w = src.width,
      .h = src.height,
      .d = src.depth,
  };
  SDL_DownloadFromGPUTexture(this->copyPass, &region, &transferInfo);
}

void CopyPass::uploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool cycle) {
  SDL_GPUTransferBufferLocation transferInfo = {
      .transfer_buffer =
          downCast<TransferBuffer>(src.transferBuffer)->getNative(),
      .offset = src.offset};
  SDL_GPUBufferRegion region = {.buffer =
                                    downCast<Buffer>(dst.buffer)->getNative(),
                                .offset = dst.offset,
                                .size = dst.size};
  SDL_UploadToGPUBuffer(this->copyPass, &transferInfo, &region, cycle);
}

void CopyPass::downloadBuffer(const BufferRegion &src,
                              const BufferTransferInfo &dst) {
  SDL_GPUBufferRegion region = {.buffer =
                                    downCast<Buffer>(src.buffer)->getNative(),
                                .offset = src.offset,
                                .size = src.size};
  SDL_GPUTransferBufferLocation transferInfo = {
      .transfer_buffer =
          downCast<TransferBuffer>(dst.transferBuffer)->getNative(),
      .offset = dst.offset,
  };
  SDL_DownloadFromGPUBuffer(this->copyPass, &region, &transferInfo);
}

void CopyPass::copyTexture(const TextureLocation &src,
                           const TextureLocation &dst, uint32 width,
                           uint32 height, uint32 depth, bool cycle) {
  SDL_GPUTextureLocation srcLocation = {
      .texture = downCast<Texture>(src.texture)->getNative(),
      .mip_level = src.mipLevel,
      .layer = src.layer,
      .x = src.x,
      .y = src.y,
      .z = src.z,
  };
  SDL_GPUTextureLocation dstLocation = {
      .texture = downCast<Texture>(dst.texture)->getNative(),
      .mip_level = dst.mipLevel,
      .layer = dst.layer,
      .x = dst.x,
      .y = dst.y,
      .z = dst.z,
  };
  SDL_CopyGPUTextureToTexture(this->copyPass, &srcLocation, &dstLocation, width,
                              height, depth, cycle);
}
} // namespace sinen::rhi::sdlgpu
#endif // EMSCRIPTEN
