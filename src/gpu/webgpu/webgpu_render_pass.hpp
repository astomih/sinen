#ifndef EMSCRIPTEN
#ifndef SINEN_WEBGPU_RENDER_PASS_HPP
#define SINEN_WEBGPU_RENDER_PASS_HPP

#include <gpu/gpu_render_pass.hpp>
#include "webgpu_command_buffer.hpp"
#include <unordered_map>
#include <vector>
#include <webgpu/webgpu.h>

namespace sinen::gpu::webgpu {
class RenderPass : public gpu::RenderPass {
public:
  RenderPass(Allocator *allocator, CommandBuffer &commandBuffer,
             WGPURenderPassEncoder renderPass,
             std::vector<WGPUTextureView> transientViews)
      : gpu::RenderPass(), allocator(allocator), commandBuffer(commandBuffer),
        renderPass(renderPass), transientViews(std::move(transientViews)),
        closed(false) {}
  ~RenderPass() override;

  WGPURenderPassEncoder getNative() const { return renderPass; }
  void close();

  void bindGraphicsPipeline(Ptr<gpu::GraphicsPipeline> pipeline) override;
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
  void applyBindings();
  void applyUniformBindings(
      UInt32 groupIndex, const std::unordered_map<UInt32, UniformBinding> &bindings);

  Allocator *allocator;
  CommandBuffer &commandBuffer;
  WGPURenderPassEncoder renderPass;
  std::vector<WGPUTextureView> transientViews;
  bool closed;
  Ptr<gpu::GraphicsPipeline> currentPipeline;
  std::unordered_map<UInt32, TextureSamplerBinding> fragmentSamplerBindings;
};
} // namespace sinen::gpu::webgpu

#endif // SINEN_WEBGPU_RENDER_PASS_HPP
#endif // EMSCRIPTEN
