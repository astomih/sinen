#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_RENDER_PASS_HPP
#define SINEN_SDLGPU_RENDER_PASS_HPP

#include <graphics/rhi/rhi_render_pass.hpp>
#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
class CommandBuffer;

class RenderPass : public rhi::RenderPass {
public:
  RenderPass(Allocator *allocator, CommandBuffer &commandBuffer,
             SDL_GPURenderPass *renderPass)
      : rhi::RenderPass(), allocator(allocator), commandBuffer(commandBuffer),
        renderPass(renderPass) {}

  SDL_GPURenderPass *getNative() const { return renderPass; }

  void bindGraphicsPipeline(Ptr<rhi::GraphicsPipeline> pipeline) override;
  void bindVertexBuffers(uint32 startSlot,
                         const Array<BufferBinding> &bindings) override;
  void bindIndexBuffer(const BufferBinding &binding,
                       IndexElementSize indexElementSize) override;
  void bindFragmentSamplers(
      uint32 startSlot, const Array<TextureSamplerBinding> &bindings) override;
  void setViewport(const Viewport &viewport) override;
  void setScissor(int32 x, int32 y, int32 width, int32 height) override;
  void drawPrimitives(uint32 vertexCount, uint32 instanceCount,
                      uint32 firstVertex, uint32 firstInstance) override;
  void drawIndexedPrimitives(uint32 indexCount, uint32 instanceCount,
                             uint32 firstIndex, uint32 vertexOffset,
                             uint32 firstInstance) override;

private:
  Allocator *allocator;
  SDL_GPURenderPass *renderPass;
  CommandBuffer &commandBuffer;
};
} // namespace sinen::rhi::sdlgpu

#endif // SINEN_SDLGPU_RENDER_PASS_HPP
#endif // EMSCRIPTEN
