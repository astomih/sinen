#pragma once
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <vector>

#include <SDL_vulkan.h>
#include <math/vector2.hpp>
#include <vulkan/vk_layer.h>

namespace sinen {
class vk_swapchain {
public:
  vk_swapchain(VkInstance instance, VkDevice device, VkSurfaceKHR surface);
  ~vk_swapchain();

  void prepare(VkPhysicalDevice physDev, uint32_t graphicsQueueIndex,
               uint32_t width, uint32_t height, VkFormat desireFormat);
  void cleanup();

  VkResult acquire_next_image(uint32_t *pImageIndex, VkSemaphore semaphore,
                              uint64_t timeout = UINT64_MAX);
  void queue_present(VkQueue queue, uint32_t imageIndex,
                     VkSemaphore waitRenderComplete);

  VkSurfaceFormatKHR GetSurfaceFormat() const { return m_selectFormat; }

  VkExtent2D GetSurfaceExtent() const { return m_surfaceExtent; }
  uint32_t GetImageCount() const { return uint32_t(m_images.size()); }
  VkImageView GetImageView(int index) { return m_imageViews[index]; }
  VkImage GetImage(int index) { return m_images[index]; };

  VkSurfaceKHR GetSurface() const { return m_surface; }

  bool is_need_recreate(const vector2 &current_size) {
    return (GetSurfaceExtent().width != current_size.x ||
            GetSurfaceExtent().height != current_size.y);
  }

private:
  VkSwapchainKHR m_swapchain;
  VkSurfaceKHR m_surface;
  VkInstance m_vkInstance;
  VkDevice m_device;
  VkSurfaceCapabilitiesKHR m_surfaceCaps;

  std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
  VkSurfaceFormatKHR m_selectFormat;
  VkExtent2D m_surfaceExtent;
  VkPresentModeKHR m_presentMode;

  std::vector<VkImage> m_images;
  std::vector<VkImageView> m_imageViews;
};
} // namespace sinen
#endif
