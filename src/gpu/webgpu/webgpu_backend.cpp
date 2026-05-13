#include "webgpu_backend.hpp"

#include "webgpu_api.hpp"
#include "webgpu_device.hpp"
#include <SDL3/SDL.h>

#ifdef SINEN_PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#endif

#include <core/logger/log.hpp>
#include <memory>

namespace sinen::gpu::webgpu {
namespace {
struct AdapterRequestState {
  bool done = false;
  bool success = false;
  WGPUAdapter adapter = nullptr;
};

struct DeviceRequestState {
  bool done = false;
  bool success = false;
  WGPUDevice device = nullptr;
};

WGPUStringView toWgpuStringView(const char *str) {
  if (!str) {
    return {nullptr, 0};
  }
  return {str, WGPU_STRLEN};
}

bool waitForRequest(WGPUInstance instance, const char *label,
                    const bool &done) {
#ifdef SINEN_PLATFORM_EMSCRIPTEN
  (void)instance;
  const double startTime = emscripten_get_now();
  constexpr double requestTimeoutMs = 10000.0;
  while (!done) {
    if (emscripten_get_now() - startTime > requestTimeoutMs) {
      return false;
    }
    emscripten_sleep(1);
  }
  return true;
#elif defined(WEBGPU_BACKEND_DAWN)
  while (!done) {
    wgpuInstanceProcessEvents(instance);
  }
  return true;
#else
  (void)instance;
  (void)label;
  (void)done;
  return true;
#endif
}

void onAdapterRequest(WGPURequestAdapterStatus status, WGPUAdapter adapter,
                      WGPUStringView message, void *userdata1,
                      void *userdata2) {
  (void)message;
  (void)userdata2;
  auto *state = static_cast<AdapterRequestState *>(userdata1);
  if (status != WGPURequestAdapterStatus_Success || !adapter) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to request WebGPU adapter: %.*s",
                 static_cast<int>(message.length), message.data);
  }
  state->done = true;
  state->success = (status == WGPURequestAdapterStatus_Success && adapter);
  state->adapter = adapter;
}

void onDeviceRequest(WGPURequestDeviceStatus status, WGPUDevice device,
                     WGPUStringView message, void *userdata1, void *userdata2) {
  (void)message;
  (void)userdata2;
  auto *state = static_cast<DeviceRequestState *>(userdata1);
  state->done = true;
  state->success = (status == WGPURequestDeviceStatus_Success && device);
  state->device = device;
}

void onDeviceLost(WGPUDevice const *device, WGPUDeviceLostReason reason,
                  WGPUStringView message, void *userdata1, void *userdata2) {
  (void)device;
  (void)userdata1;
  (void)userdata2;
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
               "WebGPU device lost: reason=%d message=%.*s",
               static_cast<int>(reason), static_cast<int>(message.length),
               message.data ? message.data : "");
}

void onUncapturedError(WGPUDevice const *device, WGPUErrorType type,
                       WGPUStringView message, void *userdata1,
                       void *userdata2) {
  (void)device;
  (void)userdata1;
  (void)userdata2;
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
               "WebGPU uncaptured error: type=%d message=%.*s",
               static_cast<int>(type), static_cast<int>(message.length),
               message.data ? message.data : "");
}
} // namespace

Ptr<gpu::Device>
Backend::createDevice(const gpu::Device::CreateInfo &createInfo) {
  (void)createInfo.debugMode;
  WGPUInstanceDescriptor instanceDesc{};
  auto instance = wgpuCreateInstance(&instanceDesc);
  if (!instance) {
    Log::error("Failed to create WebGPU instance");
    return nullptr;
  }

  auto adapterState = std::make_unique<AdapterRequestState>();
  WGPURequestAdapterOptions adapterOptions{};
  adapterOptions.backendType = WGPUBackendType_Undefined;
  WGPURequestAdapterCallbackInfo adapterCallbackInfo{};
  adapterCallbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
  adapterCallbackInfo.callback = &onAdapterRequest;
  adapterCallbackInfo.userdata1 = adapterState.get();

  wgpuInstanceRequestAdapter(instance, &adapterOptions, adapterCallbackInfo);
  if (!waitForRequest(instance, "adapter", adapterState->done)) {
    Log::error("Timed out while requesting WebGPU adapter");
    adapterState.release();
    wgpuInstanceRelease(instance);
    return nullptr;
  }
  if (!adapterState->adapter) {
    Log::error("Failed to request WebGPU adapter");
    wgpuInstanceRelease(instance);
    return nullptr;
  }

  auto deviceState = std::make_unique<DeviceRequestState>();
  WGPUDeviceDescriptor deviceDesc{};
  deviceDesc.label = toWgpuStringView("sinen-webgpu-device");
  deviceDesc.requiredFeatureCount = 0;
  deviceDesc.requiredFeatures = nullptr;
  deviceDesc.requiredLimits = nullptr;
  deviceDesc.defaultQueue.label = toWgpuStringView("sinen-webgpu-queue");
  deviceDesc.defaultQueue.nextInChain = nullptr;
  deviceDesc.deviceLostCallbackInfo = {};
  deviceDesc.deviceLostCallbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
  deviceDesc.deviceLostCallbackInfo.callback = &onDeviceLost;
  deviceDesc.uncapturedErrorCallbackInfo = {};
  deviceDesc.uncapturedErrorCallbackInfo.callback = &onUncapturedError;
  deviceDesc.uncapturedErrorCallbackInfo.userdata1 = nullptr;
  deviceDesc.uncapturedErrorCallbackInfo.userdata2 = nullptr;

  WGPURequestDeviceCallbackInfo deviceCallbackInfo{};
  deviceCallbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
  deviceCallbackInfo.callback = &onDeviceRequest;
  deviceCallbackInfo.userdata1 = deviceState.get();
  deviceCallbackInfo.userdata2 = nullptr;

  wgpuAdapterRequestDevice(adapterState->adapter, &deviceDesc,
                           deviceCallbackInfo);
  if (!waitForRequest(instance, "device", deviceState->done)) {
    Log::error("Timed out while requesting WebGPU device");
    deviceState.release();
    wgpuAdapterRelease(adapterState->adapter);
    wgpuInstanceRelease(instance);
    return nullptr;
  }
  if (!deviceState->device) {
    Log::error("Failed to request WebGPU device");
    wgpuAdapterRelease(adapterState->adapter);
    wgpuInstanceRelease(instance);
    return nullptr;
  }
  auto queue = wgpuDeviceGetQueue(deviceState->device);
  if (!queue) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get WebGPU queue");
    wgpuDeviceRelease(deviceState->device);
    wgpuAdapterRelease(adapterState->adapter);
    wgpuInstanceRelease(instance);
    return nullptr;
  }

  return makePtr<Device>(createInfo.allocator, createInfo, instance,
                         adapterState->adapter, deviceState->device, queue);
}
} // namespace sinen::gpu::webgpu
