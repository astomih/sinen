#ifndef EMSCRIPTEN
#ifndef PARANOIXA_VULKAN_RENDERER_HPP
#define PARANOIXA_VULKAN_RENDERER_HPP
// Emscripten doesn't support Vulkan
#ifndef __EMSCRIPTEN__

#include <vector>
#include <vulkan/vulkan.h>

#include "vma.hpp"

#include <array>
#include <cstdint>
#include <functional>
namespace sinen::rhi {

class VulkanRenderer {
public:
  VulkanRenderer();
  ~VulkanRenderer();
  void Initialize(void *window);
  void ProcessEvent(void *event);
  void BeginFrame();
  void EndFrame();

  void AddGuiUpdateCallBack(std::function<void()> callBack);
  struct Texture {
    Texture() = default;
    ~Texture() = default;
    VkImage image;
    VkImageView view;
    VmaAllocation allocation;
  };

private:
  void Finalize();
  void CreateInstance(void *window);
  void CreateDevice();
  void CreateSurface(void *window);
  void RecreateSwapchain(int width, int height);
  VmaVulkanFunctions GetVulkanFunctions();
  void CreateAllocator();
  void CreateCommandPool();
  void CreateDescriptorPool(VkDescriptorPool &pool);
  void CreateSemaphores();
  void CreateCommandBuffers();
  void CreateSampler();
  void CreateDescriptorSetLayout();
  void CreateDescriptorSet();
  void PrepareTriangle();
  void PrepareTexture();
  void NewFrame();
  void ProcessFrame();
  void Submit();
  VkShaderModule CreateShaderModule(const void *code, size_t length);
  void DestroyShaderModule(VkShaderModule shaderModule);
  uint32_t GetMemoryTypeIndex(VkMemoryRequirements reqs,
                              VkMemoryPropertyFlags memoryPropFlags);
  void TransitionLayoutSwapchainImage(VkCommandBuffer commandBuffer,
                                      VkImageLayout newLayout,
                                      VkAccessFlags2 newAccessFlags);
  Texture CreateTexture(const void *data, size_t size, int width, int height);
  VkImage CreateImage(uint32_t width, uint32_t height, VkFormat format,
                      VkImageUsageFlags usage, VmaMemoryUsage memoryUsage,
                      VmaAllocation &allocation,
                      VmaAllocationInfo *allocationInfo);
  void TransitionLayoutImage(VkCommandBuffer commandBuffer, VkImage image,
                             VkImageLayout oldLayout, VkImageLayout newLayout,
                             VkAccessFlags2 oldAccessFlags,
                             VkAccessFlags2 newAccessFlags,
                             VkPipelineStageFlags2 srcStageMask,
                             VkPipelineStageFlags2 dstStageMask);
  VkInstance instance;
  VkPhysicalDevice physicalDevice;
  VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
  uint32_t graphicsQueueIndex;
  VkDevice device;
  VkQueue graphicsQueue;
  VkSurfaceKHR surface;
  VkSurfaceFormatKHR surfaceFormat;
  struct SwapchainState {
    VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkAccessFlags2 accessFlags = VK_ACCESS_2_NONE;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
  };
  VkSwapchainKHR swapchain;
  std::vector<SwapchainState> swapchainState;
  VmaAllocator allocator;
  VkCommandPool commandPool;
  VkDescriptorPool descriptorPoolForImGui;
  VkDescriptorPool descriptorPool;
  VkSampler sampler;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorSet descriptorSet;
  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
  Texture texture;
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  struct Frame {
    VkSemaphore renderCompleted;
    VkSemaphore presentCompleted;
    VkFence inFlightFence;
    VkCommandBuffer commandBuffer;
  };
  struct VertexBuffer {
    VertexBuffer() : buffer(VK_NULL_HANDLE), memory(VK_NULL_HANDLE) {}
    VkBuffer buffer;
    VmaAllocation memory;
  } vertexBuffer;
  void *pWindow;
  int width, height;
  static constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;
  Frame frames[MAX_FRAMES_IN_FLIGHT];
  int currentFrameIndex = 0;
  uint32_t swapchainImageIndex = 0;

  std::vector<std::function<void()>> guiCallBacks;
};
} // namespace sinen::rhi
#endif // __EMSCRIPTEN__
#endif // PARANOIXA_VULKAN_RENDERER_HPP
#endif