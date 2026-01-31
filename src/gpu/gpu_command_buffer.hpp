#ifndef SINEN_GPU_COMMAND_BUFFER_HPP
#define SINEN_GPU_COMMAND_BUFFER_HPP
#include "gpu_copy_pass.hpp"
#include "gpu_render_pass.hpp"
namespace sinen::gpu {
struct DepthStencilTargetInfo {
  Ptr<class Texture> texture;
  float clearDepth;
  LoadOp loadOp;
  StoreOp storeOp;
  LoadOp stencilLoadOp;
  StoreOp stencilStoreOp;
  bool cycle;
  UInt8 clearStencil;
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

  virtual void pushUniformData(UInt32 slot, const void *data, size_t size) = 0;

protected:
  CommandBuffer(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};
} // namespace sinen::gpu
#endif
