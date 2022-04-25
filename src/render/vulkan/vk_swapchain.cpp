#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include "vk_swapchain.hpp"
#include "vk_base.hpp"
#include "vk_util.hpp"
#include <algorithm>

namespace nen::vk {

vk_swapchain::vk_swapchain(VkInstance instance, VkDevice device,
                           VkSurfaceKHR surface)
    : m_swapchain(VK_NULL_HANDLE), m_surface(surface), m_vkInstance(instance),
      m_device(device), m_presentMode(VK_PRESENT_MODE_IMMEDIATE_KHR) {}

vk_swapchain::~vk_swapchain() {}

void vk_swapchain::Prepare(VkPhysicalDevice physDev,
                           uint32_t graphicsQueueIndex, uint32_t width,
                           uint32_t height, VkFormat desireFormat) {
  VkResult result;
  result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDev, m_surface,
                                                     &m_surfaceCaps);
  ThrowIfFailed(result, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR Failed.");

  uint32_t count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physDev, m_surface, &count, nullptr);
  m_surfaceFormats.resize(count);
  result = vkGetPhysicalDeviceSurfaceFormatsKHR(physDev, m_surface, &count,
                                                m_surfaceFormats.data());
  ThrowIfFailed(result, "vkGetPhysicalDeviceSurfaceFormatsKHR Failed.");

  m_selectFormat = VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM,
                                      VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
  for (const auto &f : m_surfaceFormats) {
    if (f.format == desireFormat) {
      m_selectFormat = f;
      break;
    }
  }

  // サーフェース能力値の確認.
  VkBool32 isSupport;
  result = vkGetPhysicalDeviceSurfaceSupportKHR(physDev, graphicsQueueIndex,
                                                m_surface, &isSupport);
  ThrowIfFailed(result, "vkGetPhysicalDeviceSurfaceSupportKHR Failed.");
  if (isSupport == VK_FALSE) {
    throw vkutil::VulkanException(
        "vkGetPhysicalDeviceSurfaceSupportKHR: isSupport = false.");
  }

  auto imageCount = (std::max)(2u, m_surfaceCaps.minImageCount);
  auto extent = m_surfaceCaps.currentExtent;
  if (extent.width == ~0u) {
    // 値が無効のためウィンドウサイズを使用する.
    extent.width = width;
    extent.height = height;
  }
  m_surfaceExtent = extent;

  VkSwapchainKHR oldSwapchain = m_swapchain;
  uint32_t queueFamilyIndices[] = {graphicsQueueIndex};
  VkSwapchainCreateInfoKHR swapchainCI{
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      nullptr,
      0,
      m_surface,
      imageCount,
      m_selectFormat.format,
      m_selectFormat.colorSpace,
      m_surfaceExtent,
      1,
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      VK_SHARING_MODE_EXCLUSIVE,
      sizeof(queueFamilyIndices) / sizeof(queueFamilyIndices[0]),
      queueFamilyIndices,
      m_surfaceCaps.currentTransform,
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      m_presentMode,
      VK_TRUE,
      oldSwapchain};

  result = vkCreateSwapchainKHR(m_device, &swapchainCI, nullptr, &m_swapchain);
  ThrowIfFailed(result, "vkCreateSwapchainKHR Failed.");

  // 古いリソースを解放.
  if (oldSwapchain != VK_NULL_HANDLE) {
    for (auto &view : m_imageViews) {
      vkDestroyImageView(m_device, view, nullptr);
    }
    vkDestroySwapchainKHR(m_device, oldSwapchain, nullptr);
    m_imageViews.clear();
    // m_images.clear();
  }

  vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
  m_images.resize(imageCount);
  m_imageViews.resize(imageCount);
  vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_images.data());
  for (uint32_t i = 0; i < imageCount; i++) {
    VkImageViewCreateInfo viewCI{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                 nullptr,
                                 0,
                                 m_images[i],
                                 VK_IMAGE_VIEW_TYPE_2D,
                                 m_selectFormat.format,
                                 vkutil::DefaultComponentMapping(),
                                 {// VkImageSubresourceRange
                                  VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
    result = vkCreateImageView(m_device, &viewCI, nullptr, &m_imageViews[i]);
    ThrowIfFailed(result, "vkCreateImageView Failed.");
  }
}

void vk_swapchain::Cleanup() {
  if (m_device != VK_NULL_HANDLE) {
    for (auto view : m_imageViews) {
      vkDestroyImageView(m_device, view, nullptr);
    }
    if (m_swapchain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    }
    m_swapchain = VK_NULL_HANDLE;
  }

  if (m_vkInstance != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);
    m_surface = VK_NULL_HANDLE;
  }

  m_images.clear();
  m_imageViews.clear();
}

VkResult vk_swapchain::AcquireNextImage(uint32_t *pImageIndex,
                                        VkSemaphore semaphore,
                                        uint64_t timeout) {
  auto result = vkAcquireNextImageKHR(m_device, m_swapchain, timeout, semaphore,
                                      VK_NULL_HANDLE, pImageIndex);
  return result;
}

void vk_swapchain::QueuePresent(VkQueue queue, uint32_t imageIndex,
                                VkSemaphore waitRenderComplete) {
  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &m_swapchain;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &waitRenderComplete;
  vkQueuePresentKHR(queue, &presentInfo);
}

} // namespace nen::vk

#endif