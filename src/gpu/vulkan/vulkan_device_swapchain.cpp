#ifndef EMSCRIPTEN
#include "vulkan_device.hpp"
#include "vulkan_resources.hpp"

#include <SDL3/SDL.h>

#include <algorithm>

#include "volk.hpp"

namespace sinen::gpu::vulkan {
void Device::destroySwapchain() {
  swapchainTextures.clear();
  for (auto view : swapchainImageViews) {
    vkDestroyImageView(device, view, nullptr);
  }
  swapchainImageViews.clear();
  swapchainImages.clear();
  if (swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    swapchain = VK_NULL_HANDLE;
  }
}

void Device::recreateSwapchain() {
  vkDeviceWaitIdle(device);
  destroySwapchain();
  createSwapchain();
}

void Device::createSwapchain() {
  VkSurfaceCapabilitiesKHR caps{};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);

  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                       nullptr);
  std::vector<VkSurfaceFormatKHR> formats(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                       formats.data());

  VkSurfaceFormatKHR chosen = formats[0];
  for (auto &f : formats) {
    if (f.format == VK_FORMAT_B8G8R8A8_UNORM) {
      chosen = f;
      break;
    }
    if (f.format == VK_FORMAT_R8G8B8A8_UNORM) {
      chosen = f;
    }
  }
  swapchainVkFormat = chosen.format;

  uint32_t presentModeCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                            &presentModeCount, nullptr);
  std::vector<VkPresentModeKHR> modes(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                            &presentModeCount, modes.data());
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
  for (auto m : modes) {
    if (m == VK_PRESENT_MODE_MAILBOX_KHR) {
      presentMode = m;
      break;
    }
  }

  VkExtent2D extent = caps.currentExtent;
  if (extent.width == UINT32_MAX) {
    int w = 0, h = 0;
    SDL_GetWindowSize(window, &w, &h);
    extent.width = std::clamp<uint32_t>(static_cast<uint32_t>(w),
                                        caps.minImageExtent.width,
                                        caps.maxImageExtent.width);
    extent.height = std::clamp<uint32_t>(static_cast<uint32_t>(h),
                                         caps.minImageExtent.height,
                                         caps.maxImageExtent.height);
  }
  swapchainExtent = extent;

  uint32_t imageCount = caps.minImageCount + 1;
  if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
    imageCount = caps.maxImageCount;
  }

  VkSwapchainCreateInfoKHR ci{};
  ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  ci.surface = surface;
  ci.minImageCount = imageCount;
  ci.imageFormat = swapchainVkFormat;
  ci.imageColorSpace = chosen.colorSpace;
  ci.imageExtent = swapchainExtent;
  ci.imageArrayLayers = 1;
  ci.imageUsage =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  ci.preTransform = caps.currentTransform;
  ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  ci.presentMode = presentMode;
  ci.clipped = VK_TRUE;

  if (vkCreateSwapchainKHR(device, &ci, nullptr, &swapchain) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateSwapchainKHR failed");
    swapchain = VK_NULL_HANDLE;
    return;
  }

  uint32_t actualCount = 0;
  vkGetSwapchainImagesKHR(device, swapchain, &actualCount, nullptr);
  swapchainImages.resize(actualCount);
  vkGetSwapchainImagesKHR(device, swapchain, &actualCount,
                          swapchainImages.data());

  swapchainImageViews.resize(actualCount);
  swapchainTextures.resize(actualCount);

  for (uint32_t i = 0; i < actualCount; ++i) {
    VkImageViewCreateInfo viewCI{};
    viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCI.image = swapchainImages[i];
    viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCI.format = swapchainVkFormat;
    viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewCI.subresourceRange.baseMipLevel = 0;
    viewCI.subresourceRange.levelCount = 1;
    viewCI.subresourceRange.baseArrayLayer = 0;
    viewCI.subresourceRange.layerCount = 1;
    vkCreateImageView(device, &viewCI, nullptr, &swapchainImageViews[i]);

    gpu::Texture::CreateInfo tCI{};
    tCI.allocator = getCreateInfo().allocator;
    tCI.type = gpu::TextureType::Texture2D;
    tCI.format = (swapchainVkFormat == VK_FORMAT_B8G8R8A8_UNORM)
                     ? gpu::TextureFormat::B8G8R8A8_UNORM
                     : gpu::TextureFormat::R8G8B8A8_UNORM;
    tCI.usage = gpu::TextureUsage::ColorTarget;
    tCI.width = swapchainExtent.width;
    tCI.height = swapchainExtent.height;
    tCI.layerCountOrDepth = 1;
    tCI.numLevels = 1;
    tCI.sampleCount = gpu::SampleCount::x1;

    Texture::NativeCreateInfo native{};
    native.image = swapchainImages[i];
    native.view = swapchainImageViews[i];
    native.format = swapchainVkFormat;
    native.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    native.ownsImage = false;
    native.ownsView = false;
    native.allocation = VK_NULL_HANDLE;
    native.width = swapchainExtent.width;
    native.height = swapchainExtent.height;
    native.depth = 1;
    native.layers = 1;
    native.mipLevels = 1;

    swapchainTextures[i] =
        makePtr<Texture>(getCreateInfo().allocator, tCI, *this, native);
  }
}

Texture *Device::getSwapchainTexture(uint32_t index) const {
  if (index >= swapchainTextures.size()) {
    return nullptr;
  }
  return swapchainTextures[index].get();
}

VkImage Device::getSwapchainImage(uint32_t index) const {
  if (index >= swapchainImages.size()) {
    return VK_NULL_HANDLE;
  }
  return swapchainImages[index];
}
} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
