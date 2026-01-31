#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_DEVICE_HPP
#define SINEN_SDLGPU_DEVICE_HPP

#include <core/data/ptr.hpp>
#include <gpu/gpu_device.hpp>
#include <memory>

#include "sdlgpu_buffer.hpp"
#include "sdlgpu_command_buffer.hpp"
#include "sdlgpu_compute_pipeline.hpp"
#include "sdlgpu_graphics_pipeline.hpp"
#include "sdlgpu_sampler.hpp"
#include "sdlgpu_shader.hpp"
#include "sdlgpu_texture.hpp"
#include "sdlgpu_transfer_buffer.hpp"

#include <SDL3/SDL_gpu.h>

struct SDL_Window;

namespace sinen::gpu::sdlgpu {

class Device : public gpu::Device {
public:
  Device(const CreateInfo &createInfo, SDL_GPUDevice *device)
      : gpu::Device(createInfo), device(device), window(nullptr) {}
  ~Device() override;

  SDL_GPUDevice *getNative() { return device; }

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
  SDL_GPUDevice *device;
  SDL_Window *window;
};
} // namespace sinen::gpu::sdlgpu

#endif // SINEN_SDLGPU_DEVICE_HPP
#endif // EMSCRIPTEN
