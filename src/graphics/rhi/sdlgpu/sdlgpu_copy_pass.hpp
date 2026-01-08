#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_COPY_PASS_HPP
#define SINEN_SDLGPU_COPY_PASS_HPP

#include <SDL3/SDL_gpu.h>
#include <graphics/rhi/rhi_copy_pass.hpp>

namespace sinen::rhi::sdlgpu {
class CommandBuffer;
class Texture;
class TransferBuffer;
class Buffer;

class CopyPass : public rhi::CopyPass {
public:
  CopyPass(Allocator *allocator, CommandBuffer &commandBuffer,
           SDL_GPUCopyPass *copyPass)
      : rhi::CopyPass(), commandBuffer(commandBuffer), copyPass(copyPass) {}

  SDL_GPUCopyPass *getNative() { return copyPass; }

  void uploadTexture(const TextureTransferInfo &src, const TextureRegion &dst,
                     bool cycle) override;
  void downloadTexture(const TextureRegion &src,
                       const TextureTransferInfo &dst) override;
  void uploadBuffer(const BufferTransferInfo &src, const BufferRegion &dst,
                    bool cycle) override;
  void downloadBuffer(const BufferRegion &src,
                      const BufferTransferInfo &dst) override;

  void copyTexture(const TextureLocation &src, const TextureLocation &dst,
                   UInt32 width, UInt32 height, UInt32 depth,
                   bool cycle) override;

private:
  SDL_GPUCopyPass *copyPass;
  CommandBuffer &commandBuffer;
};
} // namespace sinen::rhi::sdlgpu

#endif // SINEN_SDLGPU_COPY_PASS_HPP
#endif // EMSCRIPTEN
