#ifndef EMSCRIPTEN
#include "vulkan_command.hpp"
#include "vulkan_device.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <vector>

#include "volk.hpp"

namespace sinen::gpu::vulkan {
static VkDescriptorPool
createDescriptorPool(VkDevice device, bool includeAccelerationStructures) {
  std::vector<VkDescriptorPoolSize> poolSizes;
  poolSizes.reserve(includeAccelerationStructures ? 5 : 4);
  poolSizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 512});
  poolSizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 256});
  poolSizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 256});
  poolSizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 128});
  if (includeAccelerationStructures) {
    poolSizes.push_back({VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 64});
  }

  VkDescriptorPoolCreateInfo poolCI{};
  poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolCI.maxSets = 1024;
  poolCI.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolCI.pPoolSizes = poolSizes.data();

  VkDescriptorPool pool = VK_NULL_HANDLE;
  if (vkCreateDescriptorPool(device, &poolCI, nullptr, &pool) != VK_SUCCESS) {
    return VK_NULL_HANDLE;
  }
  return pool;
}

Ptr<gpu::CommandBuffer>
Device::acquireCommandBuffer(const CommandBuffer::CreateInfo &createInfo) {
  if (deviceLost || device == VK_NULL_HANDLE || commandPool == VK_NULL_HANDLE ||
      vmaAllocator == VK_NULL_HANDLE) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: cannot acquire command buffer because device is "
                 "unavailable");
    return nullptr;
  }

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer cmd = VK_NULL_HANDLE;
  if (vkAllocateCommandBuffers(device, &allocInfo, &cmd) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkAllocateCommandBuffers failed");
    return nullptr;
  }

  VkFenceCreateInfo fenceCI{};
  fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  VkFence fence = VK_NULL_HANDLE;
  if (vkCreateFence(device, &fenceCI, nullptr, &fence) != VK_SUCCESS) {
    vkFreeCommandBuffers(device, commandPool, 1, &cmd);
    return nullptr;
  }

  VkDescriptorPool descriptorPool =
      createDescriptorPool(device, supportsRayTracing());
  if (!descriptorPool) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateDescriptorPool failed");
    vkDestroyFence(device, fence, nullptr);
    vkFreeCommandBuffers(device, commandPool, 1, &cmd);
    return nullptr;
  }

  VkDeviceSize alignment = 256;
  {
    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(physicalDevice, &props);
    alignment = props.limits.minUniformBufferOffsetAlignment;
    if (alignment == 0) {
      alignment = 256;
    }
  }

  VkDeviceSize uniformSizeBytes = 4 * 1024 * 1024;
  VkBufferCreateInfo bufferCI{};
  bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCI.size = uniformSizeBytes;
  bufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocCI{};
  allocCI.usage = VMA_MEMORY_USAGE_AUTO;
  allocCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                  VMA_ALLOCATION_CREATE_MAPPED_BIT;
  VkBuffer uniformBuffer = VK_NULL_HANDLE;
  VmaAllocation uniformAlloc = VK_NULL_HANDLE;
  VmaAllocationInfo ainfo{};
  if (vmaCreateBuffer(vmaAllocator, &bufferCI, &allocCI, &uniformBuffer,
                      &uniformAlloc, &ainfo) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vmaCreateBuffer (uniform) failed");
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyFence(device, fence, nullptr);
    vkFreeCommandBuffers(device, commandPool, 1, &cmd);
    return nullptr;
  }

  auto cb =
      makePtr<CommandBuffer>(createInfo.allocator, createInfo, *this, cmd,
                             fence, descriptorPool, uniformBuffer, uniformAlloc,
                             ainfo.pMappedData, uniformSizeBytes, alignment);
  cb->ensureRecording();
  return cb;
}

Ptr<gpu::Texture>
Device::acquireSwapchainTexture(Ptr<gpu::CommandBuffer> commandBuffer) {
  if (deviceLost || !commandBuffer) {
    return nullptr;
  }
  if (!swapchain) {
    createSwapchain();
    if (!swapchain) {
      return nullptr;
    }
  } else if (window) {
    int width = 0;
    int height = 0;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    const uint32_t drawableWidth = static_cast<uint32_t>(std::max(0, width));
    const uint32_t drawableHeight = static_cast<uint32_t>(std::max(0, height));
    if (drawableWidth == 0 || drawableHeight == 0) {
      return nullptr;
    }
    if (drawableWidth != swapchainExtent.width ||
        drawableHeight != swapchainExtent.height) {
      recreateSwapchain();
      if (!swapchain) {
        return nullptr;
      }
    }
  }
  uint32_t imageIndex = 0;
  VkResult res = vkAcquireNextImageKHR(
      device, swapchain, UINT64_MAX, VK_NULL_HANDLE, acquireFence, &imageIndex);
  if (res == VK_ERROR_DEVICE_LOST) {
    markDeviceLost("Vulkan: vkAcquireNextImageKHR failed");
    return nullptr;
  }
  if (res == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapchain();
    if (!swapchain) {
      return nullptr;
    }
    res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, VK_NULL_HANDLE,
                                acquireFence, &imageIndex);
    if (res == VK_ERROR_DEVICE_LOST) {
      markDeviceLost("Vulkan: vkAcquireNextImageKHR failed after recreate");
      return nullptr;
    }
  }
  if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkAcquireNextImageKHR failed: %d", res);
    return nullptr;
  }
  res = vkWaitForFences(device, 1, &acquireFence, VK_TRUE, UINT64_MAX);
  if (res == VK_ERROR_DEVICE_LOST) {
    markDeviceLost("Vulkan: vkWaitForFences failed during acquire");
    return nullptr;
  }
  if (res != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkWaitForFences failed during acquire: %d", res);
    return nullptr;
  }
  vkResetFences(device, 1, &acquireFence);

  auto cb = downCast<CommandBuffer>(commandBuffer);
  cb->markUsesSwapchain(imageIndex);
  return swapchainTextures[imageIndex];
}

void Device::submitCommandBuffer(Ptr<gpu::CommandBuffer> commandBuffer) {
  if (deviceLost || !commandBuffer) {
    return;
  }
  auto cb = downCast<CommandBuffer>(commandBuffer);
  cb->finishForSubmit();

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  VkCommandBuffer cmd = cb->getNative();
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd;

  VkFence fence = cb->getFence();
  vkResetFences(device, 1, &fence);
  VkResult res = vkQueueSubmit(queue, 1, &submitInfo, fence);
  if (res == VK_ERROR_DEVICE_LOST) {
    markDeviceLost("Vulkan: vkQueueSubmit failed");
    return;
  }
  if (res != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkQueueSubmit failed: %d", res);
    return;
  }
  res = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
  if (res == VK_ERROR_DEVICE_LOST) {
    markDeviceLost("Vulkan: vkWaitForFences failed during submit");
    return;
  }
  if (res != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkWaitForFences failed during submit: %d", res);
    return;
  }

  if (cb->usesSwapchain()) {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    uint32_t index = cb->getSwapchainImageIndex();
    presentInfo.pImageIndices = &index;
    VkResult pres = vkQueuePresentKHR(queue, &presentInfo);
    if (pres == VK_ERROR_DEVICE_LOST) {
      markDeviceLost("Vulkan: vkQueuePresentKHR failed");
      return;
    }
    if (pres == VK_ERROR_OUT_OF_DATE_KHR || pres == VK_SUBOPTIMAL_KHR) {
      recreateSwapchain();
    } else if (pres != VK_SUCCESS) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Vulkan: vkQueuePresentKHR failed: %d", pres);
      return;
    }
    pres = vkQueueWaitIdle(queue);
    if (pres == VK_ERROR_DEVICE_LOST) {
      markDeviceLost("Vulkan: vkQueueWaitIdle failed after present");
      return;
    }
    if (pres != VK_SUCCESS) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Vulkan: vkQueueWaitIdle failed after present: %d", pres);
    }
  }
}
} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
