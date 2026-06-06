#ifndef SINEN_WEBGPU_COMMAND_BUFFER_HPP
#define SINEN_WEBGPU_COMMAND_BUFFER_HPP

#include <gpu/gpu_command_buffer.hpp>
#include <unordered_map>
#include <utility>
#include <vector>
#include "webgpu_api.hpp"

namespace sinen::gpu::webgpu {
class Device;
class CopyPass;
class RenderPass;
class ComputePass;

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
  const std::unordered_map<UInt32, UniformBinding> &
  getComputeUniformBindings() const {
    return computeUniformBindings;
  }
  void clearDrawBindings();

  Ptr<gpu::CopyPass> beginCopyPass() override;
  void endCopyPass(Ptr<gpu::CopyPass> copyPass) override;
  Ptr<gpu::ComputePass>
  beginComputePass(const Array<StorageTextureBinding> &storageTextures,
                   const Array<StorageBufferBinding> &storageBuffers) override;
  void endComputePass(Ptr<gpu::ComputePass> computePass) override;

  Ptr<gpu::RenderPass>
  beginRenderPass(const Array<ColorTargetInfo> &infos,
                  const DepthStencilTargetInfo &depthStencilInfo, float r = 0.f,
                  float g = 0.f, float b = 0.f, float a = 1.f) override;
  void endRenderPass(Ptr<gpu::RenderPass> renderPass) override;

  void pushVertexUniformData(UInt32 slot, const void *data, Size size) override;
  void pushFragmentUniformData(UInt32 slot, const void *data,
                               Size size) override;
  void pushComputeUniformData(UInt32 slot, const void *data,
                              Size size) override;

  void releaseUniformBindings();

private:
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
  std::unordered_map<UInt32, UniformBinding> computeUniformBindings;
  std::vector<WGPUBuffer> retainedUniformBuffers;
};

class ComputePass : public gpu::ComputePass {
public:
  ComputePass(CommandBuffer &commandBuffer, WGPUComputePassEncoder pass,
              Array<StorageTextureBinding> storageTextures,
              Array<StorageBufferBinding> storageBuffers)
      : commandBuffer(commandBuffer), pass(pass),
        storageTextures(std::move(storageTextures)),
        storageBuffers(std::move(storageBuffers)) {}
  ~ComputePass() override;

  void close();
  void bindComputePipeline(Ptr<gpu::ComputePipeline> computePipeline) override;
  void dispatchWorkgroups(UInt32 groupCountX, UInt32 groupCountY,
                          UInt32 groupCountZ) override;

private:
  void bindResources();

  CommandBuffer &commandBuffer;
  WGPUComputePassEncoder pass = nullptr;
  Ptr<gpu::ComputePipeline> pipeline = nullptr;
  Array<StorageTextureBinding> storageTextures;
  Array<StorageBufferBinding> storageBuffers;
  WGPUBindGroup storageBindGroup = nullptr;
  WGPUBindGroup uniformBindGroup = nullptr;
};
} // namespace sinen::gpu::webgpu

#endif // SINEN_WEBGPU_COMMAND_BUFFER_HPP
