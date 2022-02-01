#pragma once
#include <Nen.hpp>
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#ifdef _WIN32
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vk_layer.h>
#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#endif // _WIN32
#include "Swapchain.h"
#include <Window/Window.hpp>
#include <memory>
#include <vector>
#include <vk_mem_alloc.h>

namespace nen::vk {
/**
 * @brief vulkan base framework class
 *
 */
class vulkan_base_framework {
public:
  vulkan_base_framework(class VKRenderer *);
  void initialize(std::shared_ptr<window> window);
  void terminate();
  void render();
  VkPhysicalDevice get_vk_physical_device() { return m_physDev; }
  VkDevice get_vk_device() { return m_device; }
  VkQueue get_vk_queue() { return m_deviceQueue; }
  VkCommandPool get_vk_command_pool() { return m_commandPool; }
  int32_t get_swap_buffer_count() {
    return static_cast<int32_t>(mSwapchain->GetImageCount());
  }

  static void check_result(VkResult);

  void initialize_instance(const char *appName);
  void select_physical_device();
  uint32_t search_graphics_queue_index();
  void create_device();
  void create_command_pool();
  void create_depth_buffer();
  void create_render_pass();
  void create_frame_buffer();
  void create_image_view();
  void create_semaphore();
  void create_command_buffers();

  uint32_t get_memory_type_index(uint32_t requestBits,
                                 VkMemoryPropertyFlags requestProps) const;
  void recreate_swapchain();

  std::unique_ptr<Swapchain> mSwapchain;

  VkInstance m_instance;
  VkDevice m_device;
  VkPhysicalDevice m_physDev;

  VkPhysicalDeviceMemoryProperties m_physMemProps;

  uint32_t m_graphicsQueueIndex;
  VkQueue m_deviceQueue;

  VkCommandPool m_commandPool;

  VkImage m_depthBuffer;
  VmaAllocation m_depthBufferAllocation{};
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

  std::vector<VkDeviceMemory> destroyMemory;

  uint32_t m_imageIndex;
  std::shared_ptr<window> m_window;
  VKRenderer *m_vkrenderer;
};
} // namespace nen::vk

#endif // EMSCRIPTEN