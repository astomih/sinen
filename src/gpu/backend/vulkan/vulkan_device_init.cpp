#ifndef EMSCRIPTEN
#include "vulkan_convert.hpp"
#include "vulkan_device.hpp"

#include <algorithm>
#include <cstring>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace sinen::gpu::vulkan {
static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
              VkDebugUtilsMessageTypeFlagsEXT /*type*/,
              const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
              void * /*userData*/) {
  if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Vulkan: %s",
                callbackData->pMessage);
  } else {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Vulkan: %s",
                 callbackData->pMessage);
  }
  return VK_FALSE;
}

bool hasExtension(const Array<VkExtensionProperties> &extensions,
                  const char *name) {
  return std::any_of(extensions.begin(), extensions.end(),
                     [name](const VkExtensionProperties &e) {
                       return std::strcmp(e.extensionName, name) == 0;
                     });
}

bool queryRayTracingSupport(VkPhysicalDevice pd,
                            const Array<VkExtensionProperties> &extensions) {
  if (!hasExtension(extensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) ||
      !hasExtension(extensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) ||
      !hasExtension(extensions, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) ||
      !hasExtension(extensions,
                    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)) {
    return false;
  }

  VkPhysicalDeviceBufferDeviceAddressFeatures bda{};
  bda.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
  VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructure{};
  accelerationStructure.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
  accelerationStructure.pNext = &bda;
  VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipeline{};
  rayTracingPipeline.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
  rayTracingPipeline.pNext = &accelerationStructure;
  VkPhysicalDeviceFeatures2 features2{};
  features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
  features2.pNext = &rayTracingPipeline;
  vkGetPhysicalDeviceFeatures2(pd, &features2);
  return rayTracingPipeline.rayTracingPipeline &&
         accelerationStructure.accelerationStructure && bda.bufferDeviceAddress;
}

bool queryRayQuerySupport(VkPhysicalDevice pd,
                          const Array<VkExtensionProperties> &extensions) {
  if (!hasExtension(extensions, VK_KHR_RAY_QUERY_EXTENSION_NAME)) {
    return false;
  }
  VkPhysicalDeviceRayQueryFeaturesKHR rayQuery{};
  rayQuery.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
  VkPhysicalDeviceFeatures2 features2{};
  features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
  features2.pNext = &rayQuery;
  vkGetPhysicalDeviceFeatures2(pd, &features2);
  return rayQuery.rayQuery == VK_TRUE;
}

Device::Device(const CreateInfo &createInfo) : gpu::Device(createInfo) {}

Device::~Device() {
  if (device != VK_NULL_HANDLE && !deviceLost) {
    vkDeviceWaitIdle(device);
  }

  defaultSamplerObject.reset();
  defaultTexture.reset();

  destroySwapchain();

  if (acquireFence != VK_NULL_HANDLE) {
    vkDestroyFence(device, acquireFence, nullptr);
  }
  if (commandPool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(device, commandPool, nullptr);
  }
  if (vmaAllocator != VK_NULL_HANDLE) {
    vmaDestroyAllocator(vmaAllocator);
  }
  if (device != VK_NULL_HANDLE) {
    vkDestroyDevice(device, nullptr);
  }
  if (surface != VK_NULL_HANDLE && window) {
    SDL_Vulkan_DestroySurface(instance, surface, nullptr);
  }
  if (debugMessenger != VK_NULL_HANDLE) {
    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
  }
  if (instance != VK_NULL_HANDLE) {
    vkDestroyInstance(instance, nullptr);
  }
  initialized = false;
}

void Device::disableRayTracing(const char *reason) {
  if (reason) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s; ray tracing disabled",
                reason);
  }
  rayTracingSupported = false;
  rayQuerySupported = false;
  rayTracingPipelineProperties = {};
}

void Device::markDeviceLost(const char *context) {
  if (deviceLost) {
    return;
  }
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s; Vulkan device is lost",
               context);
  deviceLost = true;
  disableRayTracing("Vulkan: device is lost");
  destroySwapchain();
}

void Device::claimWindow(void *windowPtr) {
  if (initialized) {
    return;
  }
  window = static_cast<SDL_Window *>(windowPtr);
  if (!window) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Vulkan: window is null");
    return;
  }
  initializeVulkan();
}

void Device::initializeVulkan() {
  if (volkInitialize() != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Vulkan: volkInitialize failed");
    return;
  }
  createInstance();
  if (!instance) {
    return;
  }
  volkLoadInstance(instance);

  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  if (!device) {
    return;
  }
  volkLoadDevice(device);
  createAllocator();
  if (!vmaAllocator) {
    disableRayTracing("Vulkan: allocator creation failed");
    return;
  }
  createCommandPool();
  if (!commandPool) {
    disableRayTracing("Vulkan: command pool creation failed");
    return;
  }

  VkFenceCreateInfo fenceCI{};
  fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  vkCreateFence(device, &fenceCI, nullptr, &acquireFence);

  VkPhysicalDeviceProperties props{};
  vkGetPhysicalDeviceProperties(physicalDevice, &props);
  uniformRange =
      std::min<VkDeviceSize>(65536, props.limits.maxUniformBufferRange);

  createSwapchain();
  createDefaultResources();
  if (!defaultSamplerObject || !defaultTexture) {
    disableRayTracing("Vulkan: default resource creation failed");
    return;
  }

  initialized = true;
}

void Device::createInstance() {
  Uint32 extCount = 0;
  const char *const *sdlExts = SDL_Vulkan_GetInstanceExtensions(&extCount);
  if (!sdlExts || extCount == 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: SDL_Vulkan_GetInstanceExtensions failed: %s",
                 SDL_GetError());
    return;
  }
  Array<const char *> extensions(sdlExts, sdlExts + extCount);
  if (getCreateInfo().debugMode) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  Array<const char *> layers;
  if (getCreateInfo().debugMode) {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    Array<VkLayerProperties> available(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, available.data());
    bool hasValidation = std::any_of(
        available.begin(), available.end(), [](const VkLayerProperties &p) {
          return std::strcmp(p.layerName, "VK_LAYER_KHRONOS_validation") == 0;
        });
    if (hasValidation) {
      layers.push_back("VK_LAYER_KHRONOS_validation");
    }
  }

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "sinen";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
  appInfo.pEngineName = "sinen";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
  appInfo.apiVersion = VK_API_VERSION_1_2;

  VkInstanceCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  ci.pApplicationInfo = &appInfo;
  ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  ci.ppEnabledExtensionNames = extensions.data();
  ci.enabledLayerCount = static_cast<uint32_t>(layers.size());
  ci.ppEnabledLayerNames = layers.data();

  if (vkCreateInstance(&ci, nullptr, &instance) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateInstance failed");
    instance = VK_NULL_HANDLE;
  }
}

void Device::setupDebugMessenger() {
  if (!getCreateInfo().debugMode) {
    return;
  }
  VkDebugUtilsMessengerCreateInfoEXT ci{};
  ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  ci.pfnUserCallback = debugCallback;
  vkCreateDebugUtilsMessengerEXT(instance, &ci, nullptr, &debugMessenger);
}

void Device::createSurface() {
  if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: SDL_Vulkan_CreateSurface failed: %s", SDL_GetError());
    surface = VK_NULL_HANDLE;
  }
}

void Device::pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Vulkan: no devices found");
    return;
  }
  Array<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  auto findQueue = [&](VkPhysicalDevice pd, uint32_t &outQf) -> bool {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &count, nullptr);
    Array<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &count, families.data());
    for (uint32_t i = 0; i < count; ++i) {
      if (!(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
        continue;
      }
      VkBool32 presentSupported = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, surface, &presentSupported);
      if (presentSupported) {
        outQf = i;
        return true;
      }
    }
    return false;
  };

  int bestScore = -1;
  for (auto pd : devices) {
    uint32_t qf = 0;
    if (!findQueue(pd, qf)) {
      continue;
    }
    uint32_t extCount = 0;
    vkEnumerateDeviceExtensionProperties(pd, nullptr, &extCount, nullptr);
    Array<VkExtensionProperties> exts(extCount);
    vkEnumerateDeviceExtensionProperties(pd, nullptr, &extCount, exts.data());
    bool hasSwapchain = hasExtension(exts, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    bool hasMaintenance1 =
        hasExtension(exts, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    bool hasRayTracing = queryRayTracingSupport(pd, exts);
    bool hasRayQuery = hasRayTracing && queryRayQuerySupport(pd, exts);
    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(pd, &features);
    if (!hasSwapchain || !hasMaintenance1 || !features.independentBlend ||
        !features.imageCubeArray || !features.depthClamp ||
        !features.shaderClipDistance || !features.drawIndirectFirstInstance ||
        !features.sampleRateShading) {
      continue;
    }
    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(pd, &props);
    int score =
        (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) ? 1000 : 100;
    if (score > bestScore) {
      bestScore = score;
      physicalDevice = pd;
      queueFamilyIndex = qf;
      rayTracingSupported = hasRayTracing;
      rayQuerySupported = hasRayQuery;
    }
  }
}

void Device::createLogicalDevice() {
  if (physicalDevice == VK_NULL_HANDLE) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: no suitable SDL_GPU-compatible device found");
    return;
  }

  if (!rayTracingSupported) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "Vulkan: ray tracing disabled; selected device does not "
                "support the required KHR ray tracing extensions/features");
  } else if (!rayQuerySupported) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "Vulkan: ray query disabled; selected device does not support "
                "VK_KHR_ray_query");
  }

  float priority = 1.0f;
  VkDeviceQueueCreateInfo queueCI{};
  queueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCI.queueFamilyIndex = queueFamilyIndex;
  queueCI.queueCount = 1;
  queueCI.pQueuePriorities = &priority;

  VkPhysicalDeviceFeatures features{};
  features.independentBlend = VK_TRUE;
  features.imageCubeArray = VK_TRUE;
  features.depthClamp = VK_TRUE;
  features.shaderClipDistance = VK_TRUE;
  features.drawIndirectFirstInstance = VK_TRUE;
  features.sampleRateShading = VK_TRUE;

  auto tryCreateDevice = [&]() {
    Array<const char *> extensions;
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    extensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);

    VkPhysicalDeviceBufferDeviceAddressFeatures bda{};
    bda.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructure{};
    accelerationStructure.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    accelerationStructure.accelerationStructure = VK_TRUE;
    accelerationStructure.pNext = &bda;
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipeline{};
    rayTracingPipeline.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    rayTracingPipeline.rayTracingPipeline = VK_TRUE;
    rayTracingPipeline.pNext = &accelerationStructure;
    VkPhysicalDeviceRayQueryFeaturesKHR rayQuery{};
    rayQuery.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.features = features;
    features2.pNext = &rayTracingPipeline;

    if (rayTracingSupported) {
      extensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
      extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
      extensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
      extensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
      bda.bufferDeviceAddress = VK_TRUE;
    }
    if (rayQuerySupported) {
      extensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
      rayQuery.rayQuery = VK_TRUE;
      rayQuery.pNext = features2.pNext;
      features2.pNext = &rayQuery;
    }

    VkDeviceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.queueCreateInfoCount = 1;
    ci.pQueueCreateInfos = &queueCI;
    ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    ci.ppEnabledExtensionNames = extensions.data();
    ci.pEnabledFeatures = rayTracingSupported ? nullptr : &features;
    ci.pNext = rayTracingSupported ? &features2 : nullptr;
    return vkCreateDevice(physicalDevice, &ci, nullptr, &device);
  };

  VkResult createResult = tryCreateDevice();
  if (createResult != VK_SUCCESS && rayTracingSupported) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "Vulkan: vkCreateDevice failed with ray tracing enabled "
                "(VkResult %d); retrying without ray tracing",
                createResult);
    rayTracingSupported = false;
    rayQuerySupported = false;
    device = VK_NULL_HANDLE;
    createResult = tryCreateDevice();
  }
  if (createResult != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateDevice failed: %d", createResult);
    device = VK_NULL_HANDLE;
    return;
  }
  vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

  if (rayTracingSupported) {
    VkPhysicalDeviceProperties2 props2{};
    props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    rayTracingPipelineProperties.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
    props2.pNext = &rayTracingPipelineProperties;
    vkGetPhysicalDeviceProperties2(physicalDevice, &props2);
  }
}

static VmaVulkanFunctions getVulkanFunctions() {
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

void Device::createAllocator() {
  auto functions = getVulkanFunctions();
  VmaAllocatorCreateInfo ci{};
  ci.instance = instance;
  ci.physicalDevice = physicalDevice;
  ci.device = device;
  ci.vulkanApiVersion = VK_API_VERSION_1_2;
  ci.pVulkanFunctions = &functions;
  if (rayTracingSupported) {
    ci.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  }
  if (vmaCreateAllocator(&ci, &vmaAllocator) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vmaCreateAllocator failed");
    vmaAllocator = VK_NULL_HANDLE;
  }
}

void Device::createCommandPool() {
  VkCommandPoolCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  ci.queueFamilyIndex = queueFamilyIndex;
  ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  if (vkCreateCommandPool(device, &ci, nullptr, &commandPool) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateCommandPool failed");
    commandPool = VK_NULL_HANDLE;
  }
}
} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
