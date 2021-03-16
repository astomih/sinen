#pragma once
#include <windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan_win32.h>
#include <vector>

class VKBase
{
public:
  VKBase(class VKRenderer *);
  void initialize(SDL_Window *window, const char *appName);
  void terminate();

  void render();
  VkPhysicalDevice GetVkPhysicalDevice() { return m_physDev; }
  VkDevice GetVkDevice() { return m_device; }
  VkQueue GetVkQueue() { return m_deviceQueue; }
  VkCommandPool GetVkCommandPool() { return m_commandPool; }
  int32_t GetSwapBufferCount() { return static_cast<int32_t>(m_swapchainViews.size()); }

  static void checkResult(VkResult);

  void initializeInstance(const char *appName);
  void selectPhysicalDevice();
  uint32_t searchGraphicsQueueIndex();
  void createDevice();
  void prepareCommandPool();
  void selectSurfaceFormat(VkFormat format);
  void createSwapchain(SDL_Window *window);
  void createDepthBuffer();
  void createViews();

  void createRenderPass();
  void createFramebuffer();

  void prepareCommandBuffers();
  void prepareSemaphores();

  uint32_t getMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestProps) const;

  void enableDebugReport();
  void disableDebugReport();

  VkInstance m_instance;
  VkDevice m_device;
  VkPhysicalDevice m_physDev;

  VkSurfaceKHR m_surface;
  VkSurfaceFormatKHR m_surfaceFormat;
  VkSurfaceCapabilitiesKHR m_surfaceCaps;

  VkPhysicalDeviceMemoryProperties m_physMemProps;

  uint32_t m_graphicsQueueIndex;
  VkQueue m_deviceQueue;

  VkCommandPool m_commandPool;
  VkPresentModeKHR m_presentMode;
  VkSwapchainKHR m_swapchain;
  VkExtent2D m_swapchainExtent;
  std::vector<VkImage> m_swapchainImages;
  std::vector<VkImageView> m_swapchainViews;

  VkImage m_depthBuffer;
  VkDeviceMemory m_depthBufferMemory;
  VkImageView m_depthBufferView;

  VkRenderPass m_renderPass;
  std::vector<VkFramebuffer> m_framebuffers;

  std::vector<VkFence> m_fences;
  VkSemaphore m_renderCompletedSem, m_presentCompletedSem;

  PFN_vkCreateDebugReportCallbackEXT m_vkCreateDebugReportCallbackEXT;
  PFN_vkDebugReportMessageEXT m_vkDebugReportMessageEXT;
  PFN_vkDestroyDebugReportCallbackEXT m_vkDestroyDebugReportCallbackEXT;
  VkDebugReportCallbackEXT m_debugReport;

  std::vector<VkCommandBuffer> m_commands;

  uint32_t m_imageIndex;
  SDL_Window *m_window;
  VKRenderer *m_vkrenderer;
};
