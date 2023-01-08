#ifndef SINEN_VK_BASE_HPP
#define SINEN_VK_BASE_HPP
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
#include "vk_swapchain.hpp"
#include <vk_mem_alloc.h>
#include <memory>
#include <vector>
#include <window/window.hpp>

namespace sinen {
/**
 * @brief vulkan base framework class
 *
 */
class vk_base {
public:
  vk_base(class vk_renderer *);
  void initialize();
  void shutdown();
  VkPhysicalDevice get_vk_physical_device() { return m_physDev; }
  VkDevice get_vk_device() { return m_device; }
  VkQueue get_vk_queue() { return m_deviceQueue; }
  VkCommandPool get_vk_command_pool() { return m_commandPool; }
  int32_t get_swap_buffer_count() {
    return static_cast<int32_t>(mSwapchain->GetImageCount());
  }

  void select_physical_device();
  uint32_t search_graphics_queue_index();
  void enable_debug();
  void create_instance(const char *appName);
  void create_device();
  void create_command_pool();
  void create_allocator();
  void create_depth_buffer();
  void create_render_pass();
  void create_frame_buffer();
  void create_image_view();
  void create_semaphore();
  void create_command_buffers();

  void destroy_instance();
  void destroy_device();
  void destroy_command_pool();
  void destroy_allocator();
  void destroy_depth_buffer();
  void destroy_render_pass();
  void destroy_frame_buffer();
  void destroy_image_view();
  void destroy_semaphore();
  void destroy_command_buffers();

  uint32_t get_memory_type_index(uint32_t requestBits,
                                 VkMemoryPropertyFlags requestProps) const;
  void recreate_swapchain();

  std::unique_ptr<vk_swapchain> mSwapchain;

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
  uint32_t m_imageIndex;
  vk_renderer *m_vkrenderer;
};
} // namespace sinen

#endif // EMSCRIPTEN
#endif
