#if !defined(EMSCRIPTEN) && !defined(MOBILE)
// general
#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>

// internal
#include "vk_base.hpp"
#include "vk_pipeline.hpp"
#include "vk_renderer.hpp"

namespace nen::vk {

vulkan_base_framework::vulkan_base_framework(VKRenderer *vkrenderer,
                                             manager &_manager)
    : m_imageIndex(0), m_vkrenderer(vkrenderer), m_manager(_manager) {}

void vulkan_base_framework::initialize() {
  auto &w = m_manager.get_window();
  initialize_instance(w.Name().c_str());
  select_physical_device();
  m_graphicsQueueIndex = search_graphics_queue_index();
  create_device();
  create_command_pool();
  VkSurfaceKHR surface;
  SDL_Vulkan_CreateSurface((SDL_Window *)w.GetSDLWindow(), m_instance,
                           &surface);
  mSwapchain = std::make_unique<Swapchain>(m_instance, m_device, surface);
  mSwapchain->Prepare(
      m_physDev, m_graphicsQueueIndex, static_cast<uint32_t>(w.Size().x),
      static_cast<uint32_t>(w.Size().y), VK_FORMAT_B8G8R8A8_UNORM);
  create_depth_buffer();
  create_image_view();
  create_render_pass();
  create_frame_buffer();
  create_command_buffers();
  create_semaphore();
  m_vkrenderer->prepare();
}

void vulkan_base_framework::create_image_view() {

  VkImageViewCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
  ci.format = VK_FORMAT_D32_SFLOAT;
  ci.components = {
      VK_COMPONENT_SWIZZLE_R,
      VK_COMPONENT_SWIZZLE_G,
      VK_COMPONENT_SWIZZLE_B,
      VK_COMPONENT_SWIZZLE_A,
  };
  ci.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
  ci.image = m_depthBuffer;
  vkCreateImageView(m_device, &ci, nullptr, &m_depthBufferView);
}

void vulkan_base_framework::create_semaphore() {
  VkSemaphoreCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  vkCreateSemaphore(m_device, &ci, nullptr, &m_renderCompletedSem);
  vkCreateSemaphore(m_device, &ci, nullptr, &m_presentCompletedSem);
}

void vulkan_base_framework::terminate() {
  vkDeviceWaitIdle(m_device);

  vkFreeCommandBuffers(m_device, m_commandPool, uint32_t(m_commands.size()),
                       m_commands.data());
  m_commands.clear();

  vkDestroyRenderPass(m_device, m_renderPass, nullptr);
  for (auto &v : m_framebuffers) {
    vkDestroyFramebuffer(m_device, v, nullptr);
  }
  m_framebuffers.clear();

  vmaFreeMemory(m_vkrenderer->allocator, m_depthBufferAllocation);
  vkDestroyImage(m_device, m_depthBuffer, nullptr);
  vkDestroyImageView(m_device, m_depthBufferView, nullptr);

  for (auto &v : m_fences) {
    vkDestroyFence(m_device, v, nullptr);
  }
  m_fences.clear();
  vkDestroySemaphore(m_device, m_presentCompletedSem, nullptr);
  vkDestroySemaphore(m_device, m_renderCompletedSem, nullptr);

  vkDestroyCommandPool(m_device, m_commandPool, nullptr);

  mSwapchain->Cleanup();
  vkDestroyDevice(m_device, nullptr);
  vkDestroyInstance(m_instance, nullptr);
}

void vulkan_base_framework::initialize_instance(const char *appName) {
  std::vector<const char *> extensions;
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = appName;
  appInfo.pEngineName = appName;
  appInfo.apiVersion = VK_API_VERSION_1_1;
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

  // Get all extensions
  std::vector<VkExtensionProperties> props;
  {
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    props.resize(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, props.data());

    for (const auto &v : props) {
      extensions.push_back(v.extensionName);
    }
  }

  VkInstanceCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  ci.enabledExtensionCount = uint32_t(extensions.size());
  ci.ppEnabledExtensionNames = extensions.data();
  ci.pApplicationInfo = &appInfo;

  // Create instance
  vkCreateInstance(&ci, nullptr, &m_instance);
}

void vulkan_base_framework::select_physical_device() {
  uint32_t devCount = 0;
  vkEnumeratePhysicalDevices(m_instance, &devCount, nullptr);
  std::vector<VkPhysicalDevice> physDevs(devCount);
  vkEnumeratePhysicalDevices(m_instance, &devCount, physDevs.data());

  // Use first device
  m_physDev = physDevs[0];
  vkGetPhysicalDeviceMemoryProperties(m_physDev, &m_physMemProps);
}

uint32_t vulkan_base_framework::search_graphics_queue_index() {
  uint32_t propCount;
  vkGetPhysicalDeviceQueueFamilyProperties(m_physDev, &propCount, nullptr);
  std::vector<VkQueueFamilyProperties> props(propCount);
  vkGetPhysicalDeviceQueueFamilyProperties(m_physDev, &propCount, props.data());

  uint32_t graphicsQueue = ~0u;
  for (uint32_t i = 0; i < propCount; ++i) {
    if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphicsQueue = i;
      break;
    }
  }
  return graphicsQueue;
}
void vulkan_base_framework::create_device() {
  const float defaultQueuePriority(1.0f);
  VkDeviceQueueCreateInfo devQueueCI{};
  devQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  devQueueCI.queueFamilyIndex = m_graphicsQueueIndex;
  devQueueCI.queueCount = 1;
  devQueueCI.pQueuePriorities = &defaultQueuePriority;

  std::vector<VkExtensionProperties> devExtProps;
  {
    // Get extension
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(m_physDev, nullptr, &count, nullptr);
    devExtProps.resize(count);
    vkEnumerateDeviceExtensionProperties(m_physDev, nullptr, &count,
                                         devExtProps.data());
  }

  std::vector<const char *> extensions;
  for (const auto &v : devExtProps) {
    extensions.push_back(v.extensionName);
  }
  VkDeviceCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  ci.pQueueCreateInfos = &devQueueCI;
  ci.queueCreateInfoCount = 1;
  ci.ppEnabledExtensionNames = extensions.data();
  ci.enabledExtensionCount = uint32_t(extensions.size());

  vkCreateDevice(m_physDev, &ci, nullptr, &m_device);

  vkGetDeviceQueue(m_device, m_graphicsQueueIndex, 0, &m_deviceQueue);
}

void vulkan_base_framework::create_command_pool() {
  VkCommandPoolCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  ci.queueFamilyIndex = m_graphicsQueueIndex;
  ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  vkCreateCommandPool(m_device, &ci, nullptr, &m_commandPool);
}

void vulkan_base_framework::create_depth_buffer() {
  VmaAllocatorCreateInfo allocator_info = {};
  allocator_info.physicalDevice = get_vk_physical_device();
  allocator_info.device = get_vk_device();
  vmaCreateAllocator(&allocator_info, &m_vkrenderer->allocator);
  VkImageCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  ci.imageType = VK_IMAGE_TYPE_2D;
  ci.format = VK_FORMAT_D32_SFLOAT;
  ci.extent.width = mSwapchain->GetSurfaceExtent().width;
  ci.extent.height = mSwapchain->GetSurfaceExtent().height;
  ci.extent.depth = 1;
  ci.mipLevels = 1;
  ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  ci.samples = VK_SAMPLE_COUNT_1_BIT;
  ci.arrayLayers = 1;
  VmaAllocationCreateInfo alloc_info = {};
  alloc_info.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
  vmaCreateImage(m_vkrenderer->allocator, &ci, &alloc_info, &m_depthBuffer,
                 &m_depthBufferAllocation, nullptr);

  VkMemoryRequirements reqs;
  vkGetImageMemoryRequirements(m_device, m_depthBuffer, &reqs);
  VmaAllocationCreateInfo ai{};
  ai.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
  VmaAllocationInfo allocInfo;
  allocInfo.size = reqs.size;

  vmaAllocateMemory(m_vkrenderer->allocator, &reqs, &ai,
                    &m_depthBufferAllocation, &allocInfo);
  vmaBindImageMemory(m_vkrenderer->allocator, m_depthBufferAllocation,
                     m_depthBuffer);
}

void vulkan_base_framework::create_render_pass() {
  VkRenderPassCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

  std::array<VkAttachmentDescription, 2> attachments;
  auto &colorTarget = attachments[0];
  auto &depthTarget = attachments[1];

  colorTarget = VkAttachmentDescription{};
  colorTarget.format = mSwapchain->GetSurfaceFormat().format;
  colorTarget.samples = VK_SAMPLE_COUNT_1_BIT;
  colorTarget.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorTarget.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorTarget.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorTarget.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorTarget.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorTarget.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  depthTarget = VkAttachmentDescription{};
  depthTarget.format = VK_FORMAT_D32_SFLOAT;
  depthTarget.samples = VK_SAMPLE_COUNT_1_BIT;
  depthTarget.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthTarget.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depthTarget.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthTarget.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthTarget.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthTarget.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorReference{}, depthReference{};
  colorReference.attachment = 0;
  colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  depthReference.attachment = 1;
  depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpassDesc{};
  subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDesc.colorAttachmentCount = 1;
  subpassDesc.pColorAttachments = &colorReference;
  subpassDesc.pDepthStencilAttachment = &depthReference;

  ci.attachmentCount = uint32_t(attachments.size());
  ci.pAttachments = attachments.data();
  ci.subpassCount = 1;
  ci.pSubpasses = &subpassDesc;

  vkCreateRenderPass(m_device, &ci, nullptr, &m_renderPass);
}

void vulkan_base_framework::create_frame_buffer() {
  VkFramebufferCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  ci.renderPass = m_renderPass;
  ci.width = mSwapchain->GetSurfaceExtent().width;
  ci.height = mSwapchain->GetSurfaceExtent().height;
  ci.layers = 1;
  m_framebuffers.clear();
  for (int i = 0; i < mSwapchain->GetImageCount(); i++) {
    std::array<VkImageView, 2> attachments;
    ci.attachmentCount = uint32_t(attachments.size());
    ci.pAttachments = attachments.data();
    attachments[0] = mSwapchain->GetImageView(i);
    attachments[1] = m_depthBufferView;

    VkFramebuffer framebuffer;
    vkCreateFramebuffer(m_device, &ci, nullptr, &framebuffer);
    m_framebuffers.push_back(framebuffer);
  }
}
void vulkan_base_framework::create_command_buffers() {
  VkCommandBufferAllocateInfo ai{};
  ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  ai.commandPool = m_commandPool;
  ai.commandBufferCount = mSwapchain->GetImageCount();
  ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  m_commands.resize(ai.commandBufferCount);
  vkAllocateCommandBuffers(m_device, &ai, m_commands.data());

  m_fences.resize(ai.commandBufferCount);
  VkFenceCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  for (auto &v : m_fences) {
    vkCreateFence(m_device, &ci, nullptr, &v);
  }
}

uint32_t vulkan_base_framework::get_memory_type_index(
    uint32_t requestBits, VkMemoryPropertyFlags requestProps) const {
  uint32_t result = ~0u;
  for (uint32_t i = 0; i < m_physMemProps.memoryTypeCount; ++i) {
    if (requestBits & 1) {
      const auto &types = m_physMemProps.memoryTypes[i];
      if ((types.propertyFlags & requestProps) == requestProps) {
        result = i;
        break;
      }
    }
    requestBits >>= 1;
  }
  return result;
}

void vulkan_base_framework::recreate_swapchain() {
  vkDeviceWaitIdle(m_device);

  auto size = m_manager.get_window().Size();
  mSwapchain->Prepare(m_physDev, m_graphicsQueueIndex,
                      static_cast<uint32_t>(size.x),
                      static_cast<uint32_t>(size.y), VK_FORMAT_B8G8R8A8_UNORM);
  vkDestroyImage(m_device, m_depthBuffer, nullptr);
  for (auto &v : m_framebuffers) {
    vkDestroyFramebuffer(m_device, v, nullptr);
  }
  create_depth_buffer();
  create_image_view();
  create_render_pass();
  create_frame_buffer();
}

void vulkan_base_framework::render() {
  if (mSwapchain->is_need_recreate(m_manager.get_window().Size()))
    recreate_swapchain();
  uint32_t nextImageIndex = 0;
  mSwapchain->AcquireNextImage(&nextImageIndex, m_presentCompletedSem);
  auto commandFence = m_fences[nextImageIndex];

  auto color = m_manager.get_renderer().GetClearColor();
  std::array<VkClearValue, 2> clearValue = {{
      {color.r, color.g, color.b, 1.0f}, // for Color
      {1.0f, 0}                          // for Depth
  }};

  VkRenderPassBeginInfo renderPassBI{};
  renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassBI.renderPass = m_renderPass;
  renderPassBI.framebuffer = m_framebuffers[nextImageIndex];
  renderPassBI.renderArea.offset = VkOffset2D{0, 0};
  renderPassBI.renderArea.extent = mSwapchain->GetSurfaceExtent();
  renderPassBI.pClearValues = clearValue.data();
  renderPassBI.clearValueCount = uint32_t(clearValue.size());

  // Begin Command Buffer
  VkCommandBufferBeginInfo commandBI{};
  commandBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  auto &command = m_commands[nextImageIndex];
  m_imageIndex = nextImageIndex;
  m_vkrenderer->makeCommand(command, renderPassBI, commandBI, commandFence);

  // End Render Pass
  vkCmdEndRenderPass(command);
  vkEndCommandBuffer(command);

  // Do command
  VkSubmitInfo submitInfo{};
  VkPipelineStageFlags waitStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &command;
  submitInfo.pWaitDstStageMask = &waitStageMask;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &m_presentCompletedSem;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &m_renderCompletedSem;
  vkResetFences(m_device, 1, &commandFence);
  vkQueueSubmit(m_deviceQueue, 1, &submitInfo, commandFence);
  vkQueueWaitIdle(m_deviceQueue);
  mSwapchain->QueuePresent(m_deviceQueue, nextImageIndex, m_renderCompletedSem);
}
} // namespace nen::vk
#endif