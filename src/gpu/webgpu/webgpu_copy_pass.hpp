#ifndef EMSCRIPTEN
#ifndef SINEN_WEBGPU_COPY_PASS_HPP
#define SINEN_WEBGPU_COPY_PASS_HPP

#include <gpu/gpu_copy_pass.hpp>

namespace sinen::gpu::webgpu {
class CommandBuffer;

class CopyPass : public gpu::CopyPass {
public:
  CopyPass(CommandBuffer &commandBuffer)
      : gpu::CopyPass(), commandBuffer(commandBuffer) {}

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
  CommandBuffer &commandBuffer;
};
} // namespace sinen::gpu::webgpu

#endif // SINEN_WEBGPU_COPY_PASS_HPP
#endif // EMSCRIPTEN
