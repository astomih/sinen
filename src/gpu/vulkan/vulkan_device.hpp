#pragma once
#ifndef EMSCRIPTEN
#include <gpu/gpu_device.hpp>

#include "vulkan_command.hpp"
#include "vulkan_resources.hpp"

#include "vma.hpp"

#include <SDL3/SDL_video.h>

namespace sinen::gpu::vulkan {
class Texture;

class Device : public gpu::Device {
public:
  explicit Device(const CreateInfo &createInfo);
  ~Device() override;

  void claimWindow(void *window) override;
  Ptr<gpu::Buffer> createBuffer(const Buffer::CreateInfo &createInfo) override;
  Ptr<gpu::Texture>
  createTexture(const Texture::CreateInfo &createInfo) override;
  Ptr<gpu::Sampler>
  createSampler(const Sampler::CreateInfo &createInfo) override;
  Ptr<gpu::TransferBuffer>
  createTransferBuffer(const TransferBuffer::CreateInfo &createInfo) override;
  Ptr<gpu::Shader> createShader(const Shader::CreateInfo &createInfo) override;
  Ptr<gpu::GraphicsPipeline> createGraphicsPipeline(
      const GraphicsPipeline::CreateInfo &createInfo) override;
  Ptr<gpu::ComputePipeline>
  createComputePipeline(const ComputePipeline::CreateInfo &createInfo) override;
  Ptr<gpu::CommandBuffer>
  acquireCommandBuffer(const CommandBuffer::CreateInfo &createInfo) override;
  void submitCommandBuffer(Ptr<gpu::CommandBuffer> commandBuffer) override;
  Ptr<gpu::Texture>
  acquireSwapchainTexture(Ptr<gpu::CommandBuffer> commandBuffer) override;
  gpu::TextureFormat getSwapchainFormat() const override;
  void waitForGpuIdle() override;
  String getDriver() const override;

  // Internals for backend classes.
  VkInstance getVkInstance() const { return instance; }
  VkDevice getVkDevice() const { return device; }
  VkPhysicalDevice getVkPhysicalDevice() const { return physicalDevice; }
  VkQueue getQueue() const { return queue; }
  uint32_t getQueueFamilyIndex() const { return queueFamilyIndex; }
  VmaAllocator getVmaAllocator() const { return vmaAllocator; }
  VkCommandPool getCommandPool() const { return commandPool; }
  VkDeviceSize getUniformRange() const { return uniformRange; }
  VkSampler getDefaultSampler() const { return defaultSampler; }
  VkImageView getDefaultTextureView() const { return defaultTextureView; }

  Texture *getSwapchainTexture(uint32_t index) const;
  VkImage getSwapchainImage(uint32_t index) const;

private:
  void initializeVulkan();
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createAllocator();
  void createCommandPool();
  void createSwapchain();
  void destroySwapchain();
  void recreateSwapchain();
  void createDefaultResources();

  SDL_Window *window = nullptr;
  bool initialized = false;

  VkInstance instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;
  VkQueue queue = VK_NULL_HANDLE;
  uint32_t queueFamilyIndex = 0;

  VmaAllocator vmaAllocator = VK_NULL_HANDLE;
  VkCommandPool commandPool = VK_NULL_HANDLE;

  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  VkFormat swapchainVkFormat = VK_FORMAT_UNDEFINED;
  VkExtent2D swapchainExtent{};
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;
  std::vector<Ptr<Texture>> swapchainTextures;

  VkFence acquireFence = VK_NULL_HANDLE;

  Ptr<gpu::Sampler> defaultSamplerObject;
  VkSampler defaultSampler = VK_NULL_HANDLE;
  Ptr<Texture> defaultTexture;
  VkImageView defaultTextureView = VK_NULL_HANDLE;

  VkDeviceSize uniformRange = 4096;
};
} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
