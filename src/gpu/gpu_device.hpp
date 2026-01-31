#ifndef SINEN_GPU_DEVICE_HPP
#define SINEN_GPU_DEVICE_HPP
#include "gpu_buffer.hpp"
#include "gpu_command_buffer.hpp"
#include "gpu_compute_pipeline.hpp"
#include "gpu_graphics_pipeline.hpp"
#include "gpu_transfer_buffer.hpp"

#include <core/data/string.hpp>
namespace sinen::gpu {
class Device : public std::enable_shared_from_this<Device> {
public:
  struct CreateInfo {
    Allocator *allocator;
    bool debugMode;
  };
  virtual ~Device() = default;
  const CreateInfo &getCreateInfo() const { return createInfo; }

  /**
   * @brief Claim the SDL_Window for the device
   * @param window SDL_Window pointer
   */
  virtual void claimWindow(void *window) = 0;
  virtual Ptr<Buffer> createBuffer(const Buffer::CreateInfo &createInfo) = 0;
  virtual Ptr<Texture> createTexture(const Texture::CreateInfo &createInfo) = 0;
  virtual Ptr<Sampler> createSampler(const Sampler::CreateInfo &createInfo) = 0;
  virtual Ptr<TransferBuffer>
  createTransferBuffer(const TransferBuffer::CreateInfo &createInfo) = 0;
  virtual Ptr<Shader> createShader(const Shader::CreateInfo &createInfo) = 0;
  virtual Ptr<GraphicsPipeline>
  createGraphicsPipeline(const GraphicsPipeline::CreateInfo &createInfo) = 0;
  virtual Ptr<ComputePipeline>
  createComputePipeline(const ComputePipeline::CreateInfo &createInfo) = 0;
  virtual Ptr<CommandBuffer>
  acquireCommandBuffer(const CommandBuffer::CreateInfo &createInfo) = 0;
  virtual void submitCommandBuffer(Ptr<CommandBuffer> commandBuffer) = 0;
  virtual Ptr<Texture>
  acquireSwapchainTexture(Ptr<CommandBuffer> commandBuffer) = 0;
  virtual TextureFormat getSwapchainFormat() const = 0;
  virtual void waitForGpuIdle() = 0;

  virtual String getDriver() const = 0;

protected:
  Device(const CreateInfo &createInfo) : createInfo(createInfo) {}
  Ptr<Device> getPtr() { return shared_from_this(); }

private:
  CreateInfo createInfo;
};
} // namespace sinen::gpu
#endif
