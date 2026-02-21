#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_COMMAND_BUFFER_HPP
#define SINEN_SDLGPU_COMMAND_BUFFER_HPP

#include <SDL3/SDL_gpu.h>
#include <gpu/gpu_command_buffer.hpp>

namespace sinen::gpu::sdlgpu {
class CopyPass;
class RenderPass;

class CommandBuffer : public gpu::CommandBuffer {
public:
  CommandBuffer(const CreateInfo &createInfo,
                SDL_GPUCommandBuffer *commandBuffer)
      : gpu::CommandBuffer(createInfo), commandBuffer(commandBuffer) {}

  SDL_GPUCommandBuffer *getNative() { return commandBuffer; }

  Ptr<gpu::CopyPass> beginCopyPass() override;
  void endCopyPass(Ptr<gpu::CopyPass> copyPass) override;
  Ptr<gpu::RenderPass>
  beginRenderPass(const Array<ColorTargetInfo> &infos,
                  const DepthStencilTargetInfo &depthStencilInfo, float r = 0.f,
                  float g = 0.f, float b = 0.f, float a = 1.f) override;
  void endRenderPass(Ptr<gpu::RenderPass> renderPass) override;

  void pushVertexUniformData(UInt32 slot, const void *data, Size size) override;
  void pushFragmentUniformData(UInt32 slot, const void *data,
                               Size size) override;

private:
  SDL_GPUCommandBuffer *commandBuffer;
};
} // namespace sinen::gpu::sdlgpu

#endif // SINEN_SDLGPU_COMMAND_BUFFER_HPP
#endif // EMSCRIPTEN
