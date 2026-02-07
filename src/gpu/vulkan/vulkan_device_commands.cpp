#ifndef EMSCRIPTEN
#include "vulkan_command.hpp"
#include "vulkan_device.hpp"

#include <SDL3/SDL.h>

#include <array>

#include "volk.hpp"

namespace sinen::gpu::vulkan {
static VkDescriptorPool createDescriptorPool(VkDevice device) {
  std::array<VkDescriptorPoolSize, 2> poolSizes{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  poolSizes[0].descriptorCount = 256;
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = 256;

  VkDescriptorPoolCreateInfo poolCI{};
  poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolCI.maxSets = 256;
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

  VkDescriptorPool descriptorPool = createDescriptorPool(device);
  if (!descriptorPool) {
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
  if (!swapchain) {
    createSwapchain();
  }
  uint32_t imageIndex = 0;
  VkResult res = vkAcquireNextImageKHR(
      device, swapchain, UINT64_MAX, VK_NULL_HANDLE, acquireFence, &imageIndex);
  if (res == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapchain();
    res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, VK_NULL_HANDLE,
                                acquireFence, &imageIndex);
  }
  if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkAcquireNextImageKHR failed: %d", res);
    return nullptr;
  }
  vkWaitForFences(device, 1, &acquireFence, VK_TRUE, UINT64_MAX);
  vkResetFences(device, 1, &acquireFence);

  auto cb = downCast<CommandBuffer>(commandBuffer);
  cb->markUsesSwapchain(imageIndex);
  return swapchainTextures[imageIndex];
}

void Device::submitCommandBuffer(Ptr<gpu::CommandBuffer> commandBuffer) {
  auto cb = downCast<CommandBuffer>(commandBuffer);
  cb->finishForSubmit();

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  VkCommandBuffer cmd = cb->getNative();
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd;

  VkFence fence = cb->getFence();
  vkResetFences(device, 1, &fence);
  if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Vulkan: vkQueueSubmit failed");
    return;
  }
  vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

  if (cb->usesSwapchain()) {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    uint32_t index = cb->getSwapchainImageIndex();
    presentInfo.pImageIndices = &index;
    VkResult pres = vkQueuePresentKHR(queue, &presentInfo);
    if (pres == VK_ERROR_OUT_OF_DATE_KHR || pres == VK_SUBOPTIMAL_KHR) {
      recreateSwapchain();
    }
    vkQueueWaitIdle(queue);
  }
}
} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
