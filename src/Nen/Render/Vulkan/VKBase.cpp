#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include "VKBase.h"
#include "Pipeline.h"
#include "VKRenderer.h"
#include <Nen.hpp>
#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>

namespace nen::vk {
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

  return ret;
}

void VKBase::checkResult(VkResult result) {
  if (result != VK_SUCCESS) {
  }
}

VKBase::VKBase(VKRenderer *vkrenderer)
    : m_imageIndex(0), m_vkrenderer(vkrenderer) {}

void VKBase::initialize(std::shared_ptr<window> window) {
  m_window = window;
  // Vulkan インスタンスの生成
  initializeInstance(window->Name().c_str());
  // 物理デバイスの選択
  selectPhysicalDevice();
  m_graphicsQueueIndex = searchGraphicsQueueIndex();

#ifdef _DEBUG
  // デバッグレポート関数のセット.
  enableDebugReport();
#endif

  // 論理デバイスの生成
  createDevice();
  // コマンドプールの準備
  prepareCommandPool();

  VkSurfaceKHR surface;
  // サーフェース生成
  SDL_Vulkan_CreateSurface((SDL_Window *)window->GetSDLWindow(), m_instance,
                           &surface);
  mSwapchain = std::make_unique<Swapchain>(m_instance, m_device, surface);
  mSwapchain->Prepare(
      m_physDev, m_graphicsQueueIndex, static_cast<uint32_t>(window->Size().x),
      static_cast<uint32_t>(window->Size().y), VK_FORMAT_B8G8R8A8_UNORM);

  createDepthBuffer();
  {
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
    auto result = vkCreateImageView(m_device, &ci, nullptr, &m_depthBufferView);
    checkResult(result);
  }
  createRenderPass();

  // フレームバッファの生成
  createFramebuffer();

  // コマンドバッファの準備.
  prepareCommandBuffers();
  // 描画フレーム同期用
  VkSemaphoreCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  vkCreateSemaphore(m_device, &ci, nullptr, &m_renderCompletedSem);
  vkCreateSemaphore(m_device, &ci, nullptr, &m_presentCompletedSem);

  m_vkrenderer->prepare();
}

void VKBase::terminate() {
  vkDeviceWaitIdle(m_device);

  vkFreeCommandBuffers(m_device, m_commandPool, uint32_t(m_commands.size()),
                       m_commands.data());
  m_commands.clear();

  vkDestroyRenderPass(m_device, m_renderPass, nullptr);
  for (auto &v : m_framebuffers) {
    vkDestroyFramebuffer(m_device, v, nullptr);
  }
  m_framebuffers.clear();

  vkFreeMemory(m_device, m_depthBufferMemory, nullptr);
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
#ifdef _DEBUG
  disableDebugReport();
#endif
  vkDestroyInstance(m_instance, nullptr);
}

void VKBase::initializeInstance(const char *appName) {
  std::vector<const char *> extensions;
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = appName;
  appInfo.pEngineName = appName;
  appInfo.apiVersion = VK_API_VERSION_1_1;
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

  // 拡張情報の取得.
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
#ifdef _DEBUG
  // デバッグビルド時には検証レイヤーを有効化
  const char *layers[] = {"VK_LAYER_LUNARG_standard_validation"};
  ci.enabledLayerCount = 1;
  ci.ppEnabledLayerNames = layers;
#endif

  // インスタンス生成
  auto result = vkCreateInstance(&ci, nullptr, &m_instance);
  checkResult(result);
}

void VKBase::selectPhysicalDevice() {
  uint32_t devCount = 0;
  vkEnumeratePhysicalDevices(m_instance, &devCount, nullptr);
  std::vector<VkPhysicalDevice> physDevs(devCount);
  vkEnumeratePhysicalDevices(m_instance, &devCount, physDevs.data());

  // 最初のデバイスを使用する
  m_physDev = physDevs[0];
  // メモリプロパティを取得しておく
  vkGetPhysicalDeviceMemoryProperties(m_physDev, &m_physMemProps);
}

uint32_t VKBase::searchGraphicsQueueIndex() {
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
void VKBase::createDevice() {
  const float defaultQueuePriority(1.0f);
  VkDeviceQueueCreateInfo devQueueCI{};
  devQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  devQueueCI.queueFamilyIndex = m_graphicsQueueIndex;
  devQueueCI.queueCount = 1;
  devQueueCI.pQueuePriorities = &defaultQueuePriority;

  std::vector<VkExtensionProperties> devExtProps;
  {
    // 拡張情報の取得.
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

  auto result = vkCreateDevice(m_physDev, &ci, nullptr, &m_device);
  checkResult(result);

  // デバイスキューの取得
  vkGetDeviceQueue(m_device, m_graphicsQueueIndex, 0, &m_deviceQueue);
}

void VKBase::prepareCommandPool() {
  VkCommandPoolCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  ci.queueFamilyIndex = m_graphicsQueueIndex;
  ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  auto result = vkCreateCommandPool(m_device, &ci, nullptr, &m_commandPool);
  checkResult(result);
}

void VKBase::createDepthBuffer() {
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
  auto result = vkCreateImage(m_device, &ci, nullptr, &m_depthBuffer);
  checkResult(result);

  VkMemoryRequirements reqs;
  vkGetImageMemoryRequirements(m_device, m_depthBuffer, &reqs);
  VkMemoryAllocateInfo ai{};
  ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  ai.allocationSize = reqs.size;
  ai.memoryTypeIndex = getMemoryTypeIndex(reqs.memoryTypeBits,
                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  vkAllocateMemory(m_device, &ai, nullptr, &m_depthBufferMemory);
  vkBindImageMemory(m_device, m_depthBuffer, m_depthBufferMemory, 0);
}

void VKBase::createRenderPass() {
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

  auto result = vkCreateRenderPass(m_device, &ci, nullptr, &m_renderPass);
  checkResult(result);
}

void VKBase::createFramebuffer() {
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
    auto result = vkCreateFramebuffer(m_device, &ci, nullptr, &framebuffer);
    checkResult(result);
    m_framebuffers.push_back(framebuffer);
  }
}
void VKBase::prepareCommandBuffers() {
  VkCommandBufferAllocateInfo ai{};
  ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  ai.commandPool = m_commandPool;
  ai.commandBufferCount = mSwapchain->GetImageCount();
  ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  m_commands.resize(ai.commandBufferCount);
  auto result = vkAllocateCommandBuffers(m_device, &ai, m_commands.data());
  checkResult(result);

  // コマンドバッファのフェンスも同数用意する.
  m_fences.resize(ai.commandBufferCount);
  VkFenceCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  for (auto &v : m_fences) {
    result = vkCreateFence(m_device, &ci, nullptr, &v);
    checkResult(result);
  }
}

uint32_t VKBase::getMemoryTypeIndex(uint32_t requestBits,
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

void VKBase::enableDebugReport() {
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
}
void VKBase::disableDebugReport() {
  if (m_vkDestroyDebugReportCallbackEXT) {
    m_vkDestroyDebugReportCallbackEXT(m_instance, m_debugReport, nullptr);
  }
}

void VKBase::render() {
  uint32_t nextImageIndex = 0;
  mSwapchain->AcquireNextImage(&nextImageIndex, m_presentCompletedSem);
  auto commandFence = m_fences[nextImageIndex];

  auto color = m_vkrenderer->GetRenderer()->GetClearColor();
  // クリア値
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

  // コマンドバッファ・レンダーパス開始
  VkCommandBufferBeginInfo commandBI{};
  commandBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  auto &command = m_commands[nextImageIndex];
  m_imageIndex = nextImageIndex;
  m_vkrenderer->makeCommand(command, renderPassBI, commandBI, commandFence);

  // コマンド・レンダーパス終了
  vkCmdEndRenderPass(command);
  vkEndCommandBuffer(command);

  // コマンドを実行（送信)
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
  {
    VkResult result = vkResetFences(m_device, 1, &commandFence);

    if (result != VkResult::VK_SUCCESS) {
      logger::Fatal("Vulkan Error! VkResult:%d", result);
    }
  }
  {
    VkResult result =
        vkQueueSubmit(m_deviceQueue, 1, &submitInfo, commandFence);
    if (result != VkResult::VK_SUCCESS) {
      logger::Fatal("vkQueueSubmit Error! VkResult:%d", result);
    }
    result = vkQueueWaitIdle(m_deviceQueue);
    if (result != VkResult::VK_SUCCESS) {
      logger::Fatal("vkQueueWaitIdle Error! VkResult:%d", result);
    }
  }

  // Present 処理
  mSwapchain->QueuePresent(m_deviceQueue, nextImageIndex, m_renderCompletedSem);
  for (auto &memory : destroyMemory) {
    vkFreeMemory(m_device, memory, nullptr);
  }
  destroyMemory.clear();
}
} // namespace nen::vk
#endif