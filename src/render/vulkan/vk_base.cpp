// general
#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>

// internal
#include <logger/logger.hpp>
#define VK_VERSION_1_1 1
#include "vk_base.hpp"
#include "vk_renderer.hpp"
#include "vk_util.hpp"
#include <render/renderer.hpp>

#if defined(DEBUG) || defined(_DEBUG)
#define ENABLE_VALIDATION 1
#endif
namespace sinen {
#define GetInstanceProcAddr(FuncName)                                          \
  m_##FuncName = reinterpret_cast<PFN_##FuncName>(                             \
      vkGetInstanceProcAddr(m_instance, #FuncName))
static VkBool32 VKAPI_CALL DebugReportCallback(
    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objactTypes,
    uint64_t object, size_t location, int32_t messageCode,
    const char *pLayerPrefix, const char *pMessage, void *pUserData) {
  VkBool32 ret = VK_FALSE;
  if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT ||
      flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
    ret = VK_TRUE;
  }
  std::stringstream ss;
  if (pLayerPrefix) {
    ss << "[" << pLayerPrefix << "] ";
  }
  ss << pMessage << std::endl;

  std::cout << (ss.str()) << std::endl;

  return ret;
}
VkFormat vk_base::find_supported_format(const std::vector<VkFormat> &candidates,
                                        VkImageTiling tiling,
                                        VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(this->m_physDev, format, &props);

    if ((tiling == VK_IMAGE_TILING_LINEAR) &&
        ((props.linearTilingFeatures & features) == features)) {
      return format;
    } else if ((tiling == VK_IMAGE_TILING_OPTIMAL) &&
               ((props.optimalTilingFeatures & features) == features)) {
      return format;
    }
  }
  return VK_FORMAT_UNDEFINED;
}

VkFormat vk_base::find_depth_format() {
  std::vector<VkFormat> formats;
  formats.push_back(VK_FORMAT_D32_SFLOAT);
  formats.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
  formats.push_back(VK_FORMAT_D24_UNORM_S8_UINT);

  return find_supported_format(formats, VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

vk_base::vk_base(vk_renderer *vkrenderer)
    : m_imageIndex(0), m_vkrenderer(vkrenderer) {}

void vk_base::initialize() {
  create_instance(Window::name().c_str());
  select_physical_device();
  m_graphicsQueueIndex = search_graphics_queue_index();
  enable_debug();
  create_device();
  create_command_pool();
  VkSurfaceKHR surface;
  SDL_Vulkan_CreateSurface((SDL_Window *)Window::get_sdl_window(), m_instance,
                           nullptr, &surface);
  mSwapchain = std::make_unique<vk_swapchain>(m_instance, m_device, surface);
  mSwapchain->prepare(
      m_physDev, m_graphicsQueueIndex, static_cast<uint32_t>(Window::size().x),
      static_cast<uint32_t>(Window::size().y), VK_FORMAT_B8G8R8A8_UNORM);
  create_allocator();
  create_depth_buffer();
  create_image_view();
  create_render_pass();
  create_frame_buffer();
  create_command_buffers();
  create_semaphore();
}

void vk_base::shutdown() {
  vkDeviceWaitIdle(m_device);
  destroy_command_buffers();
  destroy_render_pass();
  destroy_frame_buffer();
  destroy_image_view();
  destroy_depth_buffer();
  destroy_allocator();
  destroy_semaphore();
  destroy_command_pool();
  mSwapchain->cleanup();
  destroy_device();
#if ENABLE_VALIDATION
  m_vkDestroyDebugReportCallbackEXT(m_instance, m_debugReport, nullptr);
#endif
  destroy_instance();
}
void vk_base::enable_debug() {
#if ENABLE_VALIDATION
  GetInstanceProcAddr(vkCreateDebugReportCallbackEXT);
  GetInstanceProcAddr(vkDebugReportMessageEXT);
  GetInstanceProcAddr(vkDestroyDebugReportCallbackEXT);

  VkDebugReportFlagsEXT flags =
      VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

  VkDebugReportCallbackCreateInfoEXT drcCI{};
  drcCI.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  drcCI.flags = flags;
  drcCI.pfnCallback = &DebugReportCallback;
  m_vkCreateDebugReportCallbackEXT(m_instance, &drcCI, nullptr, &m_debugReport);
#endif
}

void vk_base::select_physical_device() {
  uint32_t devCount = 0;
  vkEnumeratePhysicalDevices(m_instance, &devCount, nullptr);
  if (devCount == 0) {
    Logger::critical("No physical device found");
  }
  std::vector<VkPhysicalDevice> physDevs(devCount);
  vkEnumeratePhysicalDevices(m_instance, &devCount, physDevs.data());
  m_physDev = VK_NULL_HANDLE;
  for (int i = 0; i < physDevs.size(); i++) {
    m_physDev = physDevs[i];
    vkGetPhysicalDeviceMemoryProperties(m_physDev, &m_physMemProps);
    // Print device name
    vkGetPhysicalDeviceProperties(m_physDev, &m_physDeviceProps);

    Logger::info("Physical Device index: %d", i);
    Logger::info("Vulkan version: %d.%d.%d",
                 VK_VERSION_MAJOR(m_physDeviceProps.apiVersion),
                 VK_VERSION_MINOR(m_physDeviceProps.apiVersion),
                 VK_VERSION_PATCH(m_physDeviceProps.apiVersion));
    Logger::info("Driver version: %d.%d.%d",
                 VK_VERSION_MAJOR(m_physDeviceProps.driverVersion),
                 VK_VERSION_MINOR(m_physDeviceProps.driverVersion),
                 VK_VERSION_PATCH(m_physDeviceProps.driverVersion));
    Logger::info("Vendor ID: %d", m_physDeviceProps.vendorID);
    Logger::info("Device ID: %d", m_physDeviceProps.deviceID);
    Logger::info("Device type: %d", m_physDeviceProps.deviceType);
    Logger::info("Device limits: %d",
                 m_physDeviceProps.limits.maxImageDimension2D);
    Logger::info("Physical device: %s\n", m_physDeviceProps.deviceName);

    if (m_physDeviceProps.apiVersion >= VK_API_VERSION_1_0) {
      break;
    }
  }
  if (m_physDev == VK_NULL_HANDLE) {
    Logger::critical("This device does not support Vulkan!");
  }
}

uint32_t vk_base::search_graphics_queue_index() {
  uint32_t propCount;
  vkGetPhysicalDeviceQueueFamilyProperties(m_physDev, &propCount, nullptr);
  std::vector<VkQueueFamilyProperties> props(propCount);
  vkGetPhysicalDeviceQueueFamilyProperties(m_physDev, &propCount, props.data());

  uint32_t graphicsQueue = ~0u;
  for (uint32_t i = 0; i < propCount; ++i) {
    if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT) {
      graphicsQueue = i;
      break;
    }
  }
  return graphicsQueue;
}

void vk_base::create_instance(const char *appName) {
  std::vector<const char *> extensions;
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = appName;
  appInfo.pEngineName = appName;
  appInfo.apiVersion = VK_API_VERSION_1_1;
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

  // Get all extensions
  std::vector<VkExtensionProperties> extensionProps;
  {
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    extensionProps.resize(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count,
                                           extensionProps.data());

    for (const auto &v : extensionProps) {
#ifdef __APPLE__
      if (std::string(v.extensionName) ==
          std::string(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        Logger::info("VK_KHR_portability_enumeration found");
      }
#endif

      extensions.push_back(v.extensionName);
    }
  }

  // Get all available layers
  std::vector<VkLayerProperties> layerProps;
  std::vector<const char *> layers;
  {
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    layerProps.resize(count);
    vkEnumerateInstanceLayerProperties(&count, layerProps.data());
#if ENABLE_VALIDATION
    for (int i = 0; i < layerProps.size(); i++) {
      if (std::string(layerProps[i].layerName) == "VK_LAYER_KHRONOS_validation")
        layers.push_back(layerProps[i].layerName);
    }
#endif
  }

  VkInstanceCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#ifdef __APPLE__
  // Use portability
  ci.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
  ci.enabledExtensionCount = extensions.size();
  ci.ppEnabledExtensionNames = extensions.data();
  ci.pApplicationInfo = &appInfo;
  ci.enabledLayerCount = layers.size();
  ci.ppEnabledLayerNames = layers.data();

  // Create instance
  VkResult result = vkCreateInstance(&ci, nullptr, &m_instance);
  if (result != VK_SUCCESS) {
    Logger::critical("Failed to create instance : %s",
                     vkutil::result_to_string(result));
    exit(1);
  }
}
void vk_base::create_device() {
  const float defaultQueuePriority(1.0f);
  VkDeviceQueueCreateInfo devQueueCI{};
  devQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  devQueueCI.queueFamilyIndex = m_graphicsQueueIndex;
  devQueueCI.queueCount = 1;
  devQueueCI.pQueuePriorities = &defaultQueuePriority;

  std::vector<const char *> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#ifdef __APPLE__
  if (m_physDeviceProps.apiVersion > VK_MAKE_API_VERSION(0, 1, 3, 216)) {
    extensions.push_back("VK_KHR_PORTABILITY_subset");
  }
#endif
  VkDeviceCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  ci.pQueueCreateInfos = &devQueueCI;
  ci.queueCreateInfoCount = 1;
  ci.ppEnabledExtensionNames = extensions.data();
  ci.enabledExtensionCount = uint32_t(extensions.size());
  VkResult result;
  result = vkCreateDevice(m_physDev, &ci, nullptr, &m_device);
  if (result != VK_SUCCESS) {
    Logger::critical("Failed to create device : %s",
                     vkutil::result_to_string(result));
    exit(1);
  }

  vkGetDeviceQueue(m_device, m_graphicsQueueIndex, 0, &m_deviceQueue);
}
void vk_base::create_image_view() {

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
  VkResult result;
  result = vkCreateImageView(m_device, &ci, nullptr, &m_depthBufferView);
  if (result != VK_SUCCESS) {
    Logger::critical("Failed to create image view : %s",
                     vkutil::result_to_string(result));
    exit(1);
  }
}
void vk_base::create_semaphore() {
  VkSemaphoreCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkResult result;
  result = vkCreateSemaphore(m_device, &ci, nullptr, &m_renderCompletedSem);
  if (result != VK_SUCCESS) {
    Logger::critical("Failed to create semaphore : %s",
                     vkutil::result_to_string(result));
    exit(1);
  }
  result = vkCreateSemaphore(m_device, &ci, nullptr, &m_presentCompletedSem);
  if (result != VK_SUCCESS) {
    Logger::critical("Failed to create semaphore : %s",
                     vkutil::result_to_string(result));
    exit(1);
  }
}
void vk_base::create_command_pool() {
  VkCommandPoolCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  ci.queueFamilyIndex = m_graphicsQueueIndex;
  ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  VkResult result;
  result = vkCreateCommandPool(m_device, &ci, nullptr, &m_commandPool);
  if (result != VK_SUCCESS) {
    Logger::critical("Failed to create command pool");
    exit(1);
  }
}
void vk_base::create_allocator() {
  VmaAllocatorCreateInfo allocator_info = {};
  allocator_info.physicalDevice = get_vk_physical_device();
  allocator_info.device = get_vk_device();
  allocator_info.vulkanApiVersion = VK_API_VERSION_1_1;
  allocator_info.instance = m_instance;
  VkResult result;
  result = vmaCreateAllocator(&allocator_info, &m_vkrenderer->allocator);
  if (result != VK_SUCCESS) {
    Logger::critical("Failed to create allocator : %s",
                     vkutil::result_to_string(result));
    exit(1);
  }
}
void vk_base::create_depth_buffer() {
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
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  VmaAllocationInfo alloc_info_out = {};
  VkResult result;
  result =
      vmaCreateImage(m_vkrenderer->allocator, &ci, &alloc_info, &m_depthBuffer,
                     &m_depthBufferAllocation, &alloc_info_out);
  if (result != VK_SUCCESS || m_depthBuffer == VK_NULL_HANDLE) {
    Logger::critical("Failed to create depth buffer : %s",
                     vkutil::result_to_string(result));
    exit(1);
  }
}
void vk_base::create_render_pass() {
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

  VkResult result;
  result = vkCreateRenderPass(m_device, &ci, nullptr, &m_renderPass);
  if (result != VK_SUCCESS) {
    Logger::critical("Failed to create render pass : %s",
                     vkutil::result_to_string(result));
    exit(1);
  }
}
void vk_base::create_frame_buffer() {
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
    VkResult result;
    result = vkCreateFramebuffer(m_device, &ci, nullptr, &framebuffer);
    if (result != VK_SUCCESS) {
      Logger::critical("Failed to create framebuffer : %s",
                       vkutil::result_to_string(result));
      exit(1);
    }
    m_framebuffers.push_back(framebuffer);
  }
}
void vk_base::create_command_buffers() {
  VkCommandBufferAllocateInfo ai{};
  ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  ai.commandPool = m_commandPool;
  ai.commandBufferCount = mSwapchain->GetImageCount();
  ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  m_commands.resize(ai.commandBufferCount);
  VkResult result;
  result = vkAllocateCommandBuffers(m_device, &ai, m_commands.data());
  if (result != VK_SUCCESS) {
    Logger::critical("Failed to allocate command buffers : %s",
                     vkutil::result_to_string(result));
    exit(1);
  }

  m_fences.resize(ai.commandBufferCount);
  VkFenceCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  for (auto &v : m_fences) {
    result = vkCreateFence(m_device, &ci, nullptr, &v);
    if (result != VK_SUCCESS) {
      Logger::critical("Failed to create fence : %s",
                       vkutil::result_to_string(result));
      exit(1);
    }
  }
}
void vk_base::destroy_instance() { vkDestroyInstance(m_instance, nullptr); }
void vk_base::destroy_device() { vkDestroyDevice(m_device, nullptr); }

void vk_base::destroy_command_pool() {
  vkDestroyCommandPool(m_device, m_commandPool, nullptr);
}
void vk_base::destroy_allocator() {
  vmaDestroyAllocator(m_vkrenderer->allocator);
}
void vk_base::destroy_image_view() {
  vkDestroyImageView(m_device, m_depthBufferView, nullptr);
}
void vk_base::destroy_depth_buffer() {
  vmaDestroyImage(m_vkrenderer->allocator, m_depthBuffer,
                  m_depthBufferAllocation);
}
void vk_base::destroy_render_pass() {
  vkDestroyRenderPass(m_device, m_renderPass, nullptr);
}
void vk_base::destroy_frame_buffer() {
  for (auto v : m_framebuffers) {
    vkDestroyFramebuffer(m_device, v, nullptr);
  }
  m_framebuffers.clear();
}
void vk_base::destroy_command_buffers() {
  for (auto &v : m_fences) {
    vkDestroyFence(m_device, v, nullptr);
  }
  m_fences.clear();
  vkFreeCommandBuffers(m_device, m_commandPool, uint32_t(m_commands.size()),
                       m_commands.data());
}
void vk_base::destroy_semaphore() {
  vkDestroySemaphore(m_device, m_presentCompletedSem, nullptr);
  vkDestroySemaphore(m_device, m_renderCompletedSem, nullptr);
}
uint32_t
vk_base::get_memory_type_index(uint32_t requestBits,
                               VkMemoryPropertyFlags requestProps) const {
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

void vk_base::recreate_swapchain() {
  vkDeviceWaitIdle(m_device);
  auto size = Window::size();
  auto format = mSwapchain->GetSurfaceFormat().format;
  mSwapchain->prepare(m_physDev, m_graphicsQueueIndex,
                      static_cast<uint32_t>(size.x),
                      static_cast<uint32_t>(size.y), format);

  destroy_frame_buffer();
  destroy_image_view();
  destroy_depth_buffer();

  create_depth_buffer();
  create_image_view();
  create_frame_buffer();
}
} // namespace sinen
