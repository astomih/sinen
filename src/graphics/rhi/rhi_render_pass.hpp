#ifndef SINEN_RHI_RENDER_PASS_HPP
#define SINEN_RHI_RENDER_PASS_HPP
#include "rhi_buffer.hpp"
#include "rhi_graphics_pipeline.hpp"
namespace sinen::rhi {
enum class IndexElementSize { Uint16, Uint32 };
struct TextureSamplerBinding {
  Ptr<class Sampler> sampler;
  Ptr<class Texture> texture;
};
struct Viewport {
  float x;
  float y;
  float width;
  float height;
  float minDepth;
  float maxDepth;
};
class RenderPass {
public:
  virtual ~RenderPass() = default;

  virtual void bindGraphicsPipeline(Ptr<GraphicsPipeline> graphicsPipeline) = 0;
  virtual void bindVertexBuffers(UInt32 slot,
                                 const Array<BufferBinding> &bindings) = 0;
  virtual void bindIndexBuffer(const BufferBinding &binding,
                               IndexElementSize indexElementSize) = 0;
  virtual void
  bindFragmentSamplers(UInt32 slot,
                       const Array<TextureSamplerBinding> &bindings) = 0;
  virtual void bindFragmentSampler(UInt32 startSlot,
                                   const TextureSamplerBinding &binding) = 0;
  virtual void setViewport(const Viewport &viewport) = 0;
  virtual void setScissor(Int32 x, Int32 y, Int32 width, Int32 height) = 0;
  virtual void drawPrimitives(UInt32 numVertices, UInt32 numInstances,
                              UInt32 firstVertex, UInt32 firstInstance) = 0;
  virtual void drawIndexedPrimitives(UInt32 numIndices, UInt32 numInstances,
                                     UInt32 firstIndex, UInt32 vertexOffset,
                                     UInt32 firstInstance) = 0;

protected:
  RenderPass() = default;
};
} // namespace sinen::rhi
#endif