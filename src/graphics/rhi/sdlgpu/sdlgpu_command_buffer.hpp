#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_COMMAND_BUFFER_HPP
#define SINEN_SDLGPU_COMMAND_BUFFER_HPP

#include <SDL3/SDL_gpu.h>
#include <graphics/rhi/rhi_command_buffer.hpp>

namespace sinen::rhi::sdlgpu {
class CopyPass;
class RenderPass;

class CommandBuffer : public rhi::CommandBuffer {
public:
  CommandBuffer(const CreateInfo &createInfo,
                SDL_GPUCommandBuffer *commandBuffer)
      : rhi::CommandBuffer(createInfo), commandBuffer(commandBuffer) {}

  SDL_GPUCommandBuffer *getNative() { return commandBuffer; }

  Ptr<rhi::CopyPass> beginCopyPass() override;
  void endCopyPass(Ptr<rhi::CopyPass> copyPass) override;
  Ptr<rhi::RenderPass>
  beginRenderPass(const Array<rhi::ColorTargetInfo> &infos,
                  const DepthStencilTargetInfo &depthStencilInfo, float r = 0.f,
                  float g = 0.f, float b = 0.f, float a = 1.f) override;
  void endRenderPass(Ptr<rhi::RenderPass> renderPass) override;

  void pushUniformData(UInt32 slot, const void *data, Size size) override;

private:
  SDL_GPUCommandBuffer *commandBuffer;
};
} // namespace sinen::rhi::sdlgpu

#endif // SINEN_SDLGPU_COMMAND_BUFFER_HPP
#endif // EMSCRIPTEN
