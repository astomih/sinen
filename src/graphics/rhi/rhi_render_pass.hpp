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
  virtual void bindVertexBuffers(uint32 slot,
                                 const Array<BufferBinding> &bindings) = 0;
  virtual void bindIndexBuffer(const BufferBinding &binding,
                               IndexElementSize indexElementSize) = 0;
  virtual void
  bindFragmentSamplers(uint32 slot,
                       const Array<TextureSamplerBinding> &bindings) = 0;
  virtual void setViewport(const Viewport &viewport) = 0;
  virtual void setScissor(int32 x, int32 y, int32 width, int32 height) = 0;
  virtual void drawPrimitives(uint32 numVertices, uint32 numInstances,
                              uint32 firstVertex, uint32 firstInstance) = 0;
  virtual void drawIndexedPrimitives(uint32 numIndices, uint32 numInstances,
                                     uint32 firstIndex, uint32 vertexOffset,
                                     uint32 firstInstance) = 0;

protected:
  RenderPass() = default;
};
} // namespace sinen::rhi
#endif