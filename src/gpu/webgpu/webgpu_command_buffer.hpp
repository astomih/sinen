#ifndef SINEN_WEBGPU_COMMAND_BUFFER_HPP
#define SINEN_WEBGPU_COMMAND_BUFFER_HPP

#include <gpu/gpu_command_buffer.hpp>
#include <unordered_map>
#include <webgpu/webgpu.h>

namespace sinen::gpu::webgpu {
class Device;
class CopyPass;
class RenderPass;

struct UniformBinding {
  WGPUBuffer buffer = nullptr;
  UInt64 size = 0;
};

class CommandBuffer : public gpu::CommandBuffer {
public:
  CommandBuffer(const CreateInfo &createInfo, const Ptr<Device> &device,
                WGPUCommandEncoder commandEncoder)
      : gpu::CommandBuffer(createInfo), device(device),
        commandEncoder(commandEncoder), commandBuffer(nullptr),
        shouldPresent(false), submitted(false) {}
  ~CommandBuffer() override;

  Ptr<Device> getDevice() const { return device; }
  WGPUCommandEncoder getEncoder() const { return commandEncoder; }
  WGPUCommandBuffer finish();
  bool isSubmitted() const { return submitted; }
  void setSubmitted(bool value) { submitted = value; }
  void setShouldPresent(bool value) { shouldPresent = value; }
  bool getShouldPresent() const { return shouldPresent; }
  const std::unordered_map<UInt32, UniformBinding> &
  getVertexUniformBindings() const {
    return vertexUniformBindings;
  }
  const std::unordered_map<UInt32, UniformBinding> &
  getFragmentUniformBindings() const {
    return fragmentUniformBindings;
  }

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
  void releaseUniformBindings();
  void
  updateUniformBinding(std::unordered_map<UInt32, UniformBinding> &bindings,
                       UInt32 slot, const void *data, Size size);

  Ptr<Device> device;
  WGPUCommandEncoder commandEncoder;
  WGPUCommandBuffer commandBuffer;
  bool shouldPresent;
  bool submitted;
  std::unordered_map<UInt32, UniformBinding> vertexUniformBindings;
  std::unordered_map<UInt32, UniformBinding> fragmentUniformBindings;
};
} // namespace sinen::gpu::webgpu

#endif // SINEN_WEBGPU_COMMAND_BUFFER_HPP