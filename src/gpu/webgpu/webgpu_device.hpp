#ifndef EMSCRIPTEN
#ifndef SINEN_WEBGPU_DEVICE_HPP
#define SINEN_WEBGPU_DEVICE_HPP

#include <core/data/ptr.hpp>
#include <gpu/gpu_device.hpp>
#include <memory>

#include "webgpu_buffer.hpp"
#include "webgpu_command_buffer.hpp"
#include "webgpu_compute_pipeline.hpp"
#include "webgpu_graphics_pipeline.hpp"
#include "webgpu_sampler.hpp"
#include "webgpu_shader.hpp"
#include "webgpu_texture.hpp"
#include "webgpu_transfer_buffer.hpp"

#include <webgpu/webgpu.h>

struct SDL_Window;

namespace sinen::gpu::webgpu {

class Device : public gpu::Device {
public:
  Device(const CreateInfo &createInfo, WGPUInstance instance, WGPUAdapter adapter,
         WGPUDevice device, WGPUQueue queue)
      : gpu::Device(createInfo), instance(instance), adapter(adapter),
        device(device), queue(queue), surface(nullptr), window(nullptr),
        swapchainFormat(WGPUTextureFormat_Undefined), configuredWidth(0),
        configuredHeight(0) {}
  ~Device() override;

  WGPUInstance getInstance() const { return instance; }
  WGPUAdapter getAdapter() const { return adapter; }
  WGPUDevice getNative() const { return device; }
  WGPUQueue getQueue() const { return queue; }
  WGPUSurface getSurface() const { return surface; }

  bool waitForFuture(WGPUFuture future) const;
  WGPUTextureView createTextureView(WGPUTexture texture,
                                    const gpu::Texture::CreateInfo &createInfo) const;
  WGPUTextureView createDefaultTextureView(WGPUTexture texture) const;

  void claimWindow(void *window) override;
  Ptr<gpu::Buffer> createBuffer(const Buffer::CreateInfo &createInfo) override;
  Ptr<gpu::Texture>
  createTexture(const Texture::CreateInfo &createInfo) override;
  Ptr<gpu::Sampler>
  createSampler(const Sampler::CreateInfo &createInfo) override;
  Ptr<gpu::TransferBuffer>
  createTransferBuffer(const TransferBuffer::CreateInfo &createInfo) override;
  Ptr<gpu::Shader> createShader(const Shader::CreateInfo &createInfo) override;
  Ptr<gpu::CommandBuffer>
  acquireCommandBuffer(const CommandBuffer::CreateInfo &createInfo) override;
  Ptr<gpu::GraphicsPipeline> createGraphicsPipeline(
      const GraphicsPipeline::CreateInfo &createInfo) override;
  Ptr<gpu::ComputePipeline>
  createComputePipeline(const ComputePipeline::CreateInfo &createInfo) override;
  void submitCommandBuffer(Ptr<gpu::CommandBuffer> commandBuffer) override;
  Ptr<gpu::Texture>
  acquireSwapchainTexture(Ptr<gpu::CommandBuffer> commandBuffer) override;
  gpu::TextureFormat getSwapchainFormat() const override;
  void waitForGpuIdle() override;
  String getDriver() const override;

  std::shared_ptr<Device> get() {
    return std::dynamic_pointer_cast<Device>(getPtr());
  }

private:
  void configureSurfaceIfNeeded();
  void configureSurface(UInt32 width, UInt32 height);

  WGPUInstance instance;
  WGPUAdapter adapter;
  WGPUDevice device;
  WGPUQueue queue;
  WGPUSurface surface;
  SDL_Window *window;
  WGPUTextureFormat swapchainFormat;
  UInt32 configuredWidth;
  UInt32 configuredHeight;
};
} // namespace sinen::gpu::webgpu

#endif // SINEN_WEBGPU_DEVICE_HPP
#endif // EMSCRIPTEN
