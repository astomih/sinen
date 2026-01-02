#ifndef SINEN_RHI_COMMAND_BUFFER_HPP
#define SINEN_RHI_COMMAND_BUFFER_HPP
#include "rhi_copy_pass.hpp"
#include "rhi_render_pass.hpp"
namespace sinen::rhi {
struct DepthStencilTargetInfo {
  Ptr<class Texture> texture;
  float clearDepth;
  LoadOp loadOp;
  StoreOp storeOp;
  LoadOp stencilLoadOp;
  StoreOp stencilStoreOp;
  bool cycle;
  uint8 clearStencil;
};
class CommandBuffer {
public:
  struct CreateInfo {
    Allocator *allocator;
  };
  virtual ~CommandBuffer() = default;

  inline const CreateInfo &getCreateInfo() const { return createInfo; }

  virtual Ptr<CopyPass> beginCopyPass() = 0;
  virtual void endCopyPass(Ptr<CopyPass> copyPass) = 0;

  virtual Ptr<class RenderPass>
  beginRenderPass(const Array<ColorTargetInfo> &infos,
                  const DepthStencilTargetInfo &depthStencilInfo, float r = 0.f,
                  float g = 0.f, float b = 0.f, float a = 1.f) = 0;
  virtual void endRenderPass(Ptr<RenderPass> renderPass) = 0;

  virtual void pushUniformData(uint32 slot, const void *data, size_t size) = 0;

protected:
  CommandBuffer(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};
} // namespace sinen::rhi
#endif