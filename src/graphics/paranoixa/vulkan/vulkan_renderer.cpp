#ifndef EMSCRIPTEN

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <volk.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>

#include "vulkan_renderer.hpp"

#ifndef _countof
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

namespace paranoixa {

VulkanRenderer::VulkanRenderer()
    : instance(VK_NULL_HANDLE), physicalDevice(VK_NULL_HANDLE),
      physicalDeviceMemoryProperties(), graphicsQueueIndex(0),
      device(VK_NULL_HANDLE), graphicsQueue(VK_NULL_HANDLE),
      surface(VK_NULL_HANDLE), surfaceFormat(), swapchain(VK_NULL_HANDLE),
      swapchainState(), commandPool(VK_NULL_HANDLE),
      descriptorPool(VK_NULL_HANDLE), pipelineLayout(VK_NULL_HANDLE),
      pipeline(VK_NULL_HANDLE), vertexBuffer(), width(0), height(0), frames(),
      currentFrameIndex(0), swapchainImageIndex(0) {}
VulkanRenderer::~VulkanRenderer() { Finalize(); }
void VulkanRenderer::Finalize() {
  vkDeviceWaitIdle(device);

  vkDestroySampler(device, sampler, nullptr);
  vkDestroyShaderModule(device, shaderStages[0].module, nullptr);
  vkDestroyShaderModule(device, shaderStages[1].module, nullptr);
  vkFreeDescriptorSets(device, descriptorPool, 1, &descriptorSet);
  vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
  vkDestroyImageView(device, texture.view, nullptr);
  vmaDestroyImage(allocator, texture.image, texture.allocation);
  vmaDestroyBuffer(allocator, vertexBuffer.buffer, vertexBuffer.memory);
  vertexBuffer.buffer = VK_NULL_HANDLE;
  vertexBuffer.memory = VK_NULL_HANDLE;

  for (auto &state : swapchainState) {
    vkDestroyImageView(device, state.view, nullptr);
  }

  vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
  vkDestroyPipeline(device, pipeline, nullptr);
  for (auto &frame : frames) {
    vkDestroyFence(device, frame.inFlightFence, nullptr);
    vkDestroySemaphore(device, frame.presentCompleted, nullptr);
    vkDestroySemaphore(device, frame.renderCompleted, nullptr);
  }
  vkDestroyDescriptorPool(device, descriptorPoolForImGui, nullptr);
  vkDestroyDescriptorPool(device, descriptorPool, nullptr);
  vkDestroyCommandPool(device, commandPool, nullptr);
  vmaDestroyAllocator(allocator);
  vkDestroySwapchainKHR(device, swapchain, nullptr);
  SDL_Vulkan_DestroySurface(instance, surface, nullptr);
  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);
}
void VulkanRenderer::Initialize(void *window) {
  pWindow = window;
  auto *sdlWindow = static_cast<SDL_Window *>(window);
  SDL_GetWindowSize(sdlWindow, &width, &height);
  volkInitialize();
  CreateInstance(window);
  CreateDevice();
  CreateSurface(window);

  RecreateSwapchain(width, height);
  CreateAllocator();
  CreateCommandPool();
  CreateDescriptorPool(descriptorPool);
  CreateSemaphores();
  CreateCommandBuffers();
  CreateSampler();
  // Load texture from SDL
  SDL_Surface *surface = SDL_LoadBMP("res/texture.bmp");

  std::vector<uint8_t> data;
  data.resize(surface->w * surface->h * 4);

  for (int y = 0; y < surface->h; ++y) {
    for (int x = 0; x < surface->w; ++x) {
      auto pixel =
          static_cast<uint32_t *>(surface->pixels) + y * surface->w + x;
      auto r = (*pixel & 0x00FF0000) >> 16;
      auto g = (*pixel & 0x0000FF00) >> 8;
      auto b = (*pixel & 0x000000FF);
      auto a = (*pixel & 0xFF000000) >> 24;
      auto index = (y * surface->w + x) * 4;
      data[index + 0] = r;
      data[index + 1] = g;
      data[index + 2] = b;
      data[index + 3] = a;
    }
  }
  texture = CreateTexture(data.data(), data.size(), surface->w, surface->h);
  SDL_DestroySurface(surface);
  CreateDescriptorSetLayout();
  CreateDescriptorSet();

  CreateDescriptorPool(descriptorPoolForImGui);

  PrepareTexture();
}
void VulkanRenderer::ProcessEvent(void *event) {}
void VulkanRenderer::BeginFrame() {
  int newWidth, newHeight;
  auto *sdlWindow = reinterpret_cast<SDL_Window *>(pWindow);
  SDL_GetWindowSize(sdlWindow, &newWidth, &newHeight);
  if (width != newWidth || height != newHeight) {
    this->RecreateSwapchain(newWidth, newHeight);
    width = newWidth;
    height = newHeight;
  }

  this->NewFrame();
  this->ProcessFrame();
}
void VulkanRenderer::EndFrame() {
  auto &frameInfo = frames[currentFrameIndex];
  vkEndCommandBuffer(frameInfo.commandBuffer);

  VkPipelineStageFlags waitStage{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSubmitInfo submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &frameInfo.presentCompleted,
      .pWaitDstStageMask = &waitStage,
      .commandBufferCount = 1,
      .pCommandBuffers = &frameInfo.commandBuffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &frameInfo.renderCompleted,
  };
  vkQueueSubmit(graphicsQueue, 1, &submitInfo, frameInfo.inFlightFence);

  currentFrameIndex = (++currentFrameIndex) % this->MAX_FRAMES_IN_FLIGHT;

  VkPresentInfoKHR presentInfo{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                               .waitSemaphoreCount = 1,
                               .pWaitSemaphores = &frameInfo.renderCompleted,
                               .swapchainCount = 1,
                               .pSwapchains = &swapchain,
                               .pImageIndices = &swapchainImageIndex};
  vkQueuePresentKHR(graphicsQueue, &presentInfo);
}
void VulkanRenderer::AddGuiUpdateCallBack(std::function<void()> callBack) {
  this->guiCallBacks.push_back(callBack);
}

void VulkanRenderer::NewFrame() {
  auto &frameInfo = this->frames[currentFrameIndex];
  auto fence = frameInfo.inFlightFence;
  vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
  auto res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                                   frameInfo.presentCompleted, VK_NULL_HANDLE,
                                   &swapchainImageIndex);
  if (res == VK_ERROR_OUT_OF_DATE_KHR) {
    return;
  }
  vkResetFences(device, 1, &fence);

  vkResetCommandBuffer(frameInfo.commandBuffer, 0);
  VkCommandBufferBeginInfo commandBeginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  };
  vkBeginCommandBuffer(frameInfo.commandBuffer, &commandBeginInfo);
}
void VulkanRenderer::ProcessFrame() {
  auto commandBuffer = this->frames[this->currentFrameIndex].commandBuffer;

  VkClearValue clearValue = {
      VkClearColorValue{1.0f, 0.4f, 0.0f, 1.0f},
  };

  TransitionLayoutSwapchainImage(commandBuffer,
                                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                 VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
  VkRenderingAttachmentInfo colorAttachmentInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = this->swapchainState[this->swapchainImageIndex].view,
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = clearValue,
  };
  VkRenderingInfo renderingInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea =
          {
              .extent = {static_cast<uint32_t>(width),
                         static_cast<uint32_t>(height)},
          },
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentInfo,
  };

  vkCmdBeginRendering(commandBuffer, &renderingInfo);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  VkDeviceSize vertexOffsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer,
                         vertexOffsets);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
  vkCmdDraw(commandBuffer, 6, 1, 0, 0);

  TransitionLayoutSwapchainImage(commandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                 VK_ACCESS_2_NONE);
}
void VulkanRenderer::Submit() {}

void VulkanRenderer::CreateInstance(void *window) {
  auto *sdlWindow = static_cast<SDL_Window *>(window);
  const char *appName = SDL_GetWindowTitle(sdlWindow);
  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = appName,
      .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
      .pEngineName = "Paranoixa",
      .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
      .apiVersion = VK_API_VERSION_1_3,
  };
  VkInstanceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = 0,
      .ppEnabledExtensionNames = nullptr,
  };

  std::vector<const char *> layers;
  std::vector<const char *> extensions;
  {
    uint32_t count;
    const char *const *extensionNames =
        SDL_Vulkan_GetInstanceExtensions(&count);
    std::for_each_n(extensionNames, count,
                    [&](auto v) { extensions.push_back(v); });
  }
#if 1
  layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();
  createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
  createInfo.ppEnabledLayerNames = layers.data();
  VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
  if (result == VK_SUCCESS) {
    volkLoadInstance(instance);
  }
}
void VulkanRenderer::CreateDevice() {
  //------------------------
  // Create physical device
  //------------------------
  uint32_t count = 0;
  vkEnumeratePhysicalDevices(instance, &count, nullptr);
  std::vector<VkPhysicalDevice> physicalDevices(count);
  vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data());

  // Choose first
  physicalDevice = physicalDevices[0];

  vkGetPhysicalDeviceMemoryProperties(physicalDevice,
                                      &physicalDeviceMemoryProperties);

  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilyProps(count);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count,
                                           queueFamilyProps.data());
  uint32_t gfxQueueIndex = UINT32_MAX;
  for (uint32_t i = 0; const auto &props : queueFamilyProps) {
    if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      gfxQueueIndex = i;
      break;
    }
    ++i;
  }
  assert(gfxQueueIndex != UINT32_MAX);
  graphicsQueueIndex = gfxQueueIndex;

  //------------------------
  // Create logic device
  //------------------------
  std::vector<const char *> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  VkPhysicalDeviceFeatures2 physFeatures2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
  VkPhysicalDeviceVulkan13Features vulkan13Features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  physFeatures2.pNext = &vulkan13Features;
  vulkan13Features.dynamicRendering = VK_TRUE;
  vulkan13Features.synchronization2 = VK_TRUE;
  vulkan13Features.maintenance4 = VK_TRUE;
  vkGetPhysicalDeviceFeatures2(physicalDevice, &physFeatures2);
  constexpr float queuePriorities[] = {1.f};
  VkDeviceQueueCreateInfo deviceQueueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = graphicsQueueIndex,
      .queueCount = 1,
      .pQueuePriorities = queuePriorities};
  VkDeviceCreateInfo deviceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &deviceQueueCreateInfo,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data()};
  deviceCreateInfo.pNext = &physFeatures2;
  auto result =
      vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
  if (result == VK_SUCCESS) {
    volkLoadDevice(device);
    vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
  }
}
void VulkanRenderer::CreateSurface(void *window) {
  SDL_Vulkan_CreateSurface(static_cast<SDL_Window *>(window), this->instance,
                           nullptr, &this->surface);
  // Select format
  uint32_t count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, this->surface,
                                       &count, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, this->surface,
                                       &count, formats.data());

  constexpr VkFormat desireFormats[] = {VK_FORMAT_B8G8R8A8_UNORM,
                                        VK_FORMAT_R8G8B8A8_UNORM};
  this->surfaceFormat.format = VK_FORMAT_UNDEFINED;
  bool found = false;
  for (int i = 0; i < std::size(desireFormats) && !found; ++i) {
    auto format = desireFormats[i];
    for (const auto &f : formats) {
      if (f.format == format &&
          f.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
        this->surfaceFormat = f;
        found = true;
        break;
      }
    }
  }
  assert(found);
}
uint32_t
VulkanRenderer::GetMemoryTypeIndex(VkMemoryRequirements reqs,
                                   VkMemoryPropertyFlags memoryPropFlags) {
  auto requestBits = reqs.memoryTypeBits;
  for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount;
       ++i) {
    if (requestBits & 1) {
      const auto types = physicalDeviceMemoryProperties.memoryTypes[i];
      if ((types.propertyFlags & memoryPropFlags) == memoryPropFlags) {
        return i;
      }
    }
    requestBits >>= 1;
  }
  return UINT32_MAX;
}

void VulkanRenderer::RecreateSwapchain(int width, int height) {
  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                            &surfaceCapabilities);
  VkExtent2D extent = surfaceCapabilities.currentExtent;
  if (extent.width == UINT32_MAX) {
    extent.width = std::clamp(static_cast<uint32_t>(width),
                              surfaceCapabilities.minImageExtent.width,
                              surfaceCapabilities.maxImageExtent.width);
    extent.height = std::clamp(static_cast<uint32_t>(height),
                               surfaceCapabilities.minImageExtent.height,
                               surfaceCapabilities.maxImageExtent.height);
  }
  VkSwapchainKHR oldSwapchain = this->swapchain;
  VkSwapchainCreateInfoKHR swapchainCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = surface,
      .minImageCount = surfaceCapabilities.minImageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = surfaceFormat.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .preTransform = surfaceCapabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
      .clipped = VK_TRUE,
      .oldSwapchain = oldSwapchain};
  vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &this->swapchain);

  uint32_t count = 0;
  vkGetSwapchainImagesKHR(this->device, this->swapchain, &count, nullptr);
  std::vector<VkImage> swapchainImages(count);
  vkGetSwapchainImagesKHR(this->device, this->swapchain, &count,
                          swapchainImages.data());

  std::vector<SwapchainState> swapchainState(count);

  for (auto i = 0; auto &state : swapchainState) {
    auto image = swapchainImages[i];
    VkImageViewCreateInfo imageViewCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = surfaceFormat.format,
        .components =
            {
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }};
    vkCreateImageView(device, &imageViewCI, nullptr, &state.view);
    state.image = image;
    state.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    state.accessFlags = VK_ACCESS_2_NONE;
    ++i;
  }
  swapchainState.swap(this->swapchainState);

  if (oldSwapchain != VK_NULL_HANDLE) {
    for (auto &state : swapchainState) {
      vkDestroyImageView(device, state.view, nullptr);
    }
    vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
  }

  count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
  std::vector<VkBool32> supportPresent(count);
  for (uint32_t i = 0; i < count; ++i) {
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, this->surface,
                                         &supportPresent[i]);
  }
  assert(supportPresent[graphicsQueueIndex] == VK_TRUE);
}
VmaVulkanFunctions VulkanRenderer::GetVulkanFunctions() {
  VmaVulkanFunctions vmaVulkanFunctions{};
  vmaVulkanFunctions.vkAllocateMemory = vkAllocateMemory;
  vmaVulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
  vmaVulkanFunctions.vkBindImageMemory = vkBindImageMemory;
  vmaVulkanFunctions.vkCreateBuffer = vkCreateBuffer;
  vmaVulkanFunctions.vkCreateImage = vkCreateImage;
  vmaVulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
  vmaVulkanFunctions.vkDestroyImage = vkDestroyImage;
  vmaVulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
  vmaVulkanFunctions.vkFreeMemory = vkFreeMemory;
  vmaVulkanFunctions.vkGetBufferMemoryRequirements =
      vkGetBufferMemoryRequirements;
  vmaVulkanFunctions.vkGetImageMemoryRequirements =
      vkGetImageMemoryRequirements;
  vmaVulkanFunctions.vkGetPhysicalDeviceMemoryProperties =
      vkGetPhysicalDeviceMemoryProperties;
  vmaVulkanFunctions.vkGetPhysicalDeviceProperties =
      vkGetPhysicalDeviceProperties;
  vmaVulkanFunctions.vkInvalidateMappedMemoryRanges =
      vkInvalidateMappedMemoryRanges;
  vmaVulkanFunctions.vkMapMemory = vkMapMemory;
  vmaVulkanFunctions.vkUnmapMemory = vkUnmapMemory;
  vmaVulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
  vmaVulkanFunctions.vkGetBufferMemoryRequirements2KHR =
      vkGetBufferMemoryRequirements2;
  vmaVulkanFunctions.vkGetImageMemoryRequirements2KHR =
      vkGetImageMemoryRequirements2;
  return vmaVulkanFunctions;
}
void VulkanRenderer::CreateAllocator() {
  VmaVulkanFunctions vmaVulkanFunctions = GetVulkanFunctions();
  VmaAllocatorCreateInfo allocatorCreateInfo{
      .physicalDevice = physicalDevice,
      .device = device,
      .pVulkanFunctions = &vmaVulkanFunctions,
      .instance = this->instance,
      .vulkanApiVersion = VK_API_VERSION_1_3,
  };
  assert(vmaCreateAllocator(&allocatorCreateInfo, &this->allocator) ==
         VK_SUCCESS);
}
void VulkanRenderer::CreateCommandPool() {
  VkCommandPoolCreateInfo commandPoolCreateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = graphicsQueueIndex};
  VkCommandPool commandPool;
  vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr,
                      &this->commandPool);
}
void VulkanRenderer::CreateDescriptorPool(VkDescriptorPool &pool) {
  constexpr uint32_t POOL_SIZE = 256;
  VkDescriptorPoolSize poolSizes[] = {
      {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = POOL_SIZE},
      {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
       .descriptorCount = POOL_SIZE},
      {.type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = POOL_SIZE},
      {
          .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = POOL_SIZE,
      }};
  VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
      .maxSets = 1,
      .poolSizeCount = std::size(poolSizes),
      .pPoolSizes = poolSizes};
  vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &pool);
}
void VulkanRenderer::CreateSemaphores() {
  VkSemaphoreCreateInfo semaphoreCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  for (auto &frame : frames) {
    vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr,
                      &frame.renderCompleted);
    vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr,
                      &frame.presentCompleted);
  }
}
void VulkanRenderer::CreateCommandBuffers() {
  VkFenceCreateInfo fenceCI{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };
  for (auto &frame : this->frames) {
    vkCreateFence(device, &fenceCI, nullptr, &frame.inFlightFence);
  }
  VkCommandBufferAllocateInfo commandBufferAllocateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = static_cast<uint32_t>(std::size(frames))};
  std::vector<VkCommandBuffer> commandBuffers(std::size(frames));
  vkAllocateCommandBuffers(device, &commandBufferAllocateInfo,
                           commandBuffers.data());
  for (size_t i = 0; i < std::size(frames); ++i) {
    frames[i].commandBuffer = commandBuffers[i];
  }
}
void VulkanRenderer::CreateSampler() {
  VkSamplerCreateInfo samplerCI{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .minLod = 0.0f,
      .maxLod = VK_LOD_CLAMP_NONE,
  };
  vkCreateSampler(device, &samplerCI, nullptr, &sampler);
}

void VulkanRenderer::CreateDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding binding[] = {
      {
          .binding = 0,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
  };
  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = _countof(binding),
      .pBindings = binding,
  };
  vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCI, nullptr,
                              &this->descriptorSetLayout);
}

void VulkanRenderer::CreateDescriptorSet() {
  VkDescriptorSetLayout descriptorSetLayouts[] = {this->descriptorSetLayout};
  VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptorPool,
      .descriptorSetCount = 1,
      .pSetLayouts = descriptorSetLayouts,
  };
  vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo,
                           &this->descriptorSet);

  VkDescriptorImageInfo imageInfo{
      .sampler = sampler,
      .imageView = this->texture.view,
      .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
  };

  VkWriteDescriptorSet writeDescriptorSet{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = this->descriptorSet,
      .dstBinding = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = &imageInfo,
  };

  vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void VulkanRenderer::PrepareTriangle() {
  float triangleVerts[] = {0.5f,  0.5f,  0.5f, 0, 0, 1,
                           0.0f,  -0.5f, 0.5f, 0, 1, 0,
                           -0.5f, 0.5f,  0.5f, 1, 0, 0};
  VkBufferCreateInfo bufferCI{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                              .size =
                                  static_cast<uint32_t>(sizeof(triangleVerts)),
                              .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
  VmaAllocationCreateInfo allocationCreateInfo{};
  allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocationCreateInfo.flags =
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
      VMA_ALLOCATION_CREATE_MAPPED_BIT;

  VmaAllocationInfo allocationInfo{};

  vmaCreateBuffer(allocator, &bufferCI, &allocationCreateInfo,
                  &vertexBuffer.buffer, &vertexBuffer.memory, &allocationInfo);
  if (allocationInfo.pMappedData != nullptr) {
    memcpy(allocationInfo.pMappedData, triangleVerts, sizeof(triangleVerts));
  }

  VkPipelineLayoutCreateInfo layoutCI{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &descriptorSetLayout,
  };
  vkCreatePipelineLayout(device, &layoutCI, nullptr, &pipelineLayout);

  VkVertexInputBindingDescription vertexBindingDesc{
      .binding = 0,
      .stride = static_cast<uint32_t>(sizeof(float)) * 6,
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };
  VkVertexInputAttributeDescription vertexAttribs[] = {
      {.location = 0,
       .binding = 0,
       .format = VK_FORMAT_R32G32B32_SFLOAT,
       .offset = 0},
      {.location = 1,
       .binding = 0,
       .format = VK_FORMAT_R32G32B32_SFLOAT,
       .offset = sizeof(float) * 3},
  };
  VkPipelineVertexInputStateCreateInfo vertexInput{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &vertexBindingDesc,
      .vertexAttributeDescriptionCount = 2,
      .pVertexAttributeDescriptions = vertexAttribs,
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

  VkViewport viewport{
      .x = 0,
      .y = 0,
      .width = static_cast<float>(width),
      .height = static_cast<float>(height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  VkRect2D scissor{
      .offset = {0, 0},
      .extent = {.width = static_cast<uint32_t>(width),
                 .height = static_cast<uint32_t>(height)},
  };

  VkPipelineViewportStateCreateInfo viewportState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = &viewport,
      .scissorCount = 1,
      .pScissors = &scissor,
  };

  VkPipelineRasterizationStateCreateInfo rasterizeState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .lineWidth = 1.0f,
  };

  VkPipelineMultisampleStateCreateInfo multisampleState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
  };

  VkPipelineColorBlendAttachmentState blendAttachment{
      .blendEnable = VK_TRUE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp = VK_BLEND_OP_ADD,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo blendState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &blendAttachment};

  VkPipelineDepthStencilStateCreateInfo depthStencilState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

  std::vector<char> vertexShaderSpv, fragmentShaderSpv;
  shaderStages = std::array<VkPipelineShaderStageCreateInfo, 2>{
      {{
           .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
           .stage = VK_SHADER_STAGE_VERTEX_BIT,
           .module = CreateShaderModule(vertexShaderSpv.data(),
                                        vertexShaderSpv.size()),
           .pName = "main",
       },
       {
           .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
           .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
           .module = CreateShaderModule(fragmentShaderSpv.data(),
                                        fragmentShaderSpv.size()),
           .pName = "main",
       }}};

  VkGraphicsPipelineCreateInfo pipelineCreateInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = static_cast<uint32_t>(shaderStages.size()),
      .pStages = shaderStages.data(),
      .pVertexInputState = &vertexInput,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizeState,
      .pMultisampleState = &multisampleState,
      .pDepthStencilState = &depthStencilState,
      .pColorBlendState = &blendState,
      .layout = pipelineLayout,
      .renderPass = VK_NULL_HANDLE,
  };
  VkFormat colorFormats[] = {this->surfaceFormat.format};
  VkPipelineRenderingCreateInfo renderingCI{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = colorFormats,
  };
  pipelineCreateInfo.pNext = &renderingCI;
  auto res = vkCreateGraphicsPipelines(
      device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &this->pipeline);
  if (res != VK_SUCCESS) {
  }
  for (auto &m : shaderStages) {
    DestroyShaderModule(m.module);
  }
}
void VulkanRenderer::PrepareTexture() {
  /*
 (-1, -1)  (1, -1)
    +--------+
    |        |
    |        |
    |        |
    +--------+
 (-1,  1)  (1,  1)
  */
  float triangleVerts[] = {
      -1.f, -1.f, 0.f, 0, 1, 0, 0, 1, // position, uv, color
      -1.f, 1.f,  0.f, 0, 0, 1, 0, 0, //
      1.f,  -1.f, 0.f, 1, 1, 0, 0, 1, //
      1.f,  -1.f, 0.f, 1, 1, 0, 0, 1, //
      -1.f, 1.f,  0.f, 0, 0, 1, 0, 0, //
      1.f,  1.f,  0.f, 1, 0, 1, 0, 0, //
  };
  VkBufferCreateInfo bufferCI{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                              .size =
                                  static_cast<uint32_t>(sizeof(triangleVerts)),
                              .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
  VmaAllocationCreateInfo allocationCreateInfo{};
  allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocationCreateInfo.flags =
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
      VMA_ALLOCATION_CREATE_MAPPED_BIT;

  VmaAllocationInfo allocationInfo{};

  vmaCreateBuffer(allocator, &bufferCI, &allocationCreateInfo,
                  &vertexBuffer.buffer, &vertexBuffer.memory, &allocationInfo);
  if (allocationInfo.pMappedData != nullptr) {
    memcpy(allocationInfo.pMappedData, triangleVerts, sizeof(triangleVerts));
  }

  VkPipelineLayoutCreateInfo layoutCI{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &descriptorSetLayout,
  };
  vkCreatePipelineLayout(device, &layoutCI, nullptr, &pipelineLayout);

  VkVertexInputBindingDescription vertexBindingDesc{
      .binding = 0,
      .stride = static_cast<uint32_t>(sizeof(float)) * 8,
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };
  VkVertexInputAttributeDescription vertexAttribs[] = {
      {.location = 0,
       .binding = 0,
       .format = VK_FORMAT_R32G32B32_SFLOAT,
       .offset = 0},
      {.location = 1,
       .binding = 0,
       .format = VK_FORMAT_R32G32_SFLOAT,
       .offset = sizeof(float) * 3},
      {.location = 2,
       .binding = 0,
       .format = VK_FORMAT_R32G32B32_SFLOAT,
       .offset = sizeof(float) * 5},
  };
  VkPipelineVertexInputStateCreateInfo vertexInput{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &vertexBindingDesc,
      .vertexAttributeDescriptionCount = _countof(vertexAttribs),
      .pVertexAttributeDescriptions = vertexAttribs,
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

  VkViewport viewport{
      .x = 0,
      .y = 0,
      .width = static_cast<float>(width),
      .height = static_cast<float>(height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  VkRect2D scissor{
      .offset = {0, 0},
      .extent = {.width = static_cast<uint32_t>(width),
                 .height = static_cast<uint32_t>(height)},
  };

  VkPipelineViewportStateCreateInfo viewportState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = &viewport,
      .scissorCount = 1,
      .pScissors = &scissor,
  };

  VkPipelineRasterizationStateCreateInfo rasterizeState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .lineWidth = 1.0f,
  };

  VkPipelineMultisampleStateCreateInfo multisampleState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
  };

  VkPipelineColorBlendAttachmentState blendAttachment{
      .blendEnable = VK_TRUE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp = VK_BLEND_OP_ADD,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo blendState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &blendAttachment};

  VkPipelineDepthStencilStateCreateInfo depthStencilState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

  std::vector<char> vertexShaderSpv, fragmentShaderSpv;

  shaderStages = std::array<VkPipelineShaderStageCreateInfo, 2>{
      {{
           .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
           .stage = VK_SHADER_STAGE_VERTEX_BIT,
           .module = CreateShaderModule(vertexShaderSpv.data(),
                                        vertexShaderSpv.size()),
           .pName = "main",
       },
       {
           .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
           .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
           .module = CreateShaderModule(fragmentShaderSpv.data(),
                                        fragmentShaderSpv.size()),
           .pName = "main",
       }}};

  VkGraphicsPipelineCreateInfo pipelineCreateInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = static_cast<uint32_t>(shaderStages.size()),
      .pStages = shaderStages.data(),
      .pVertexInputState = &vertexInput,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizeState,
      .pMultisampleState = &multisampleState,
      .pDepthStencilState = &depthStencilState,
      .pColorBlendState = &blendState,
      .layout = pipelineLayout,
      .renderPass = VK_NULL_HANDLE,
  };
  VkFormat colorFormats[] = {this->surfaceFormat.format};
  VkPipelineRenderingCreateInfo renderingCI{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = colorFormats,
  };
  pipelineCreateInfo.pNext = &renderingCI;
  auto res = vkCreateGraphicsPipelines(
      device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &this->pipeline);
  if (res != VK_SUCCESS) {
  }
  for (auto &m : shaderStages) {
  }
}
VkShaderModule VulkanRenderer::CreateShaderModule(const void *code,
                                                  size_t length) {
  VkShaderModuleCreateInfo shaderModuleCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = length,
      .pCode = reinterpret_cast<const uint32_t *>(code),
  };
  VkShaderModule shaderModule;
  vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule);
  return shaderModule;
}
void VulkanRenderer::DestroyShaderModule(VkShaderModule shaderModule) {
  vkDestroyShaderModule(device, shaderModule, nullptr);
}
void VulkanRenderer::TransitionLayoutSwapchainImage(
    VkCommandBuffer commandBuffer, VkImageLayout newLayout,
    VkAccessFlags2 newAccessFlags) {
  auto &swapchainState = this->swapchainState[swapchainImageIndex];
  VkImageMemoryBarrier2 barrierToRT{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .pNext = nullptr,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = swapchainState.accessFlags,
      .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = newAccessFlags,
      .oldLayout = swapchainState.layout,
      .newLayout = newLayout,
      .image = swapchainState.image,
      .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
      }};

  VkDependencyInfo info{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrierToRT,
  };

  if (vkCmdPipelineBarrier2) {
    vkCmdPipelineBarrier2(commandBuffer, &info);
  } else {
    vkCmdPipelineBarrier2KHR(commandBuffer, &info);
  }
  swapchainState.layout = newLayout;
  swapchainState.accessFlags = newAccessFlags;
}
VulkanRenderer::Texture VulkanRenderer::CreateTexture(const void *data,
                                                      size_t size, int width,
                                                      int height) {
  Texture texture;
  VmaAllocationInfo allocationInfo;
  texture.image =
      CreateImage(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                  VMA_MEMORY_USAGE_AUTO, texture.allocation, &allocationInfo);

  // Copy data
  VkBufferCreateInfo bufferCI{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
  };
  VmaAllocationCreateInfo allocationCreateInfo{};
  allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
  VmaAllocation stagingAllocation;
  VkBuffer stagingBuffer;
  vmaCreateBuffer(allocator, &bufferCI, &allocationCreateInfo, &stagingBuffer,
                  &stagingAllocation, nullptr);
  void *mappedData;
  vmaMapMemory(allocator, stagingAllocation, &mappedData);
  memcpy(mappedData, data, size);
  vmaUnmapMemory(allocator, stagingAllocation);

  VkCommandBufferAllocateInfo commandBufferAI{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };
  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device, &commandBufferAI, &commandBuffer);

  VkCommandBufferBeginInfo commandBufferBI{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  };
  vkBeginCommandBuffer(commandBuffer, &commandBufferBI);

  VkImageMemoryBarrier imageBarrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = texture.image,
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };
  /*
  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &imageBarrier);
                       */
  TransitionLayoutImage(commandBuffer, texture.image, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_2_NONE,
                        VK_ACCESS_TRANSFER_WRITE_BIT,
                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT);

  VkBufferImageCopy bufferImageCopy{

      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
      .imageOffset = {0, 0, 0},
      .imageExtent = {static_cast<uint32_t>(width),
                      static_cast<uint32_t>(height), 1},
  };
  vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, texture.image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                         &bufferImageCopy);

  imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
  TransitionLayoutImage(
      commandBuffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_TRANSFER_WRITE_BIT,
      VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT,
      VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
  };
  vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphicsQueue);

  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
  vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

  VkImageViewCreateInfo imageViewCI{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = texture.image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_UNORM,
      .components =
          {
              VK_COMPONENT_SWIZZLE_IDENTITY,
              VK_COMPONENT_SWIZZLE_IDENTITY,
              VK_COMPONENT_SWIZZLE_IDENTITY,
              VK_COMPONENT_SWIZZLE_IDENTITY,
          },
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };
  vkCreateImageView(device, &imageViewCI, nullptr, &texture.view);
  return texture;
}
VkImage VulkanRenderer::CreateImage(uint32_t width, uint32_t height,
                                    VkFormat format, VkImageUsageFlags usage,
                                    VmaMemoryUsage memoryUsage,
                                    VmaAllocation &allocation,
                                    VmaAllocationInfo *allocationInfo) {
  VkImageCreateInfo imageCI{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = format,
      .extent = {width, height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  VmaAllocationCreateInfo allocationCreateInfo{};
  allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
  VkImage image;
  if (vmaCreateImage(this->allocator, &imageCI, &allocationCreateInfo, &image,
                     &allocation, allocationInfo) != VK_SUCCESS) {
    return VK_NULL_HANDLE;
  }
  return image;
}
void VulkanRenderer::TransitionLayoutImage(
    VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout,
    VkImageLayout newLayout, VkAccessFlags2 oldAccessFlags,
    VkAccessFlags2 newAccessFlags, VkPipelineStageFlags2 srcStageMask,
    VkPipelineStageFlags2 dstStageMask) {
  VkImageMemoryBarrier2 barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .pNext = nullptr,
      .srcStageMask = srcStageMask,
      .srcAccessMask = oldAccessFlags,
      .dstStageMask = dstStageMask,
      .dstAccessMask = newAccessFlags,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .image = image,
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  VkDependencyInfo info{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };

  if (vkCmdPipelineBarrier2) {
    vkCmdPipelineBarrier2(commandBuffer, &info);
  } else {
    vkCmdPipelineBarrier2KHR(commandBuffer, &info);
  }
}
} // namespace paranoixa
#endif