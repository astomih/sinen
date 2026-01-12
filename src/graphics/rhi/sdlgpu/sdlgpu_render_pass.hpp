#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_RENDER_PASS_HPP
#define SINEN_SDLGPU_RENDER_PASS_HPP

#include <SDL3/SDL_gpu.h>
#include <graphics/rhi/rhi_render_pass.hpp>

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
  void bindVertexBuffers(UInt32 startSlot,
                         const Array<BufferBinding> &bindings) override;
  void bindIndexBuffer(const BufferBinding &binding,
                       IndexElementSize indexElementSize) override;
  void
  bindFragmentSamplers(UInt32 startSlot,
                       const Array<TextureSamplerBinding> &bindings) override;
  void bindFragmentSampler(UInt32 startSlot,
                           const TextureSamplerBinding &binding) override;
  void setViewport(const Viewport &viewport) override;
  void setScissor(Int32 x, Int32 y, Int32 width, Int32 height) override;
  void drawPrimitives(UInt32 vertexCount, UInt32 instanceCount,
                      UInt32 firstVertex, UInt32 firstInstance) override;
  void drawIndexedPrimitives(UInt32 indexCount, UInt32 instanceCount,
                             UInt32 firstIndex, UInt32 vertexOffset,
                             UInt32 firstInstance) override;

private:
  Allocator *allocator;
  SDL_GPURenderPass *renderPass;
  CommandBuffer &commandBuffer;
};
} // namespace sinen::rhi::sdlgpu

#endif // SINEN_SDLGPU_RENDER_PASS_HPP
#endif // EMSCRIPTEN
