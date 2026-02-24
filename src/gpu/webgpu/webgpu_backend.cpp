#include "webgpu_backend.hpp"

#include "webgpu_device.hpp"
#include <SDL3/SDL.h>
#include <webgpu/webgpu.h>

#include <core/logger/log.hpp>

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

void waitForRequest() {
#ifdef SINEN_PLATFORM_EMSCRIPTEN
  while (userData.adapterRequested == false) {
    emscripten_sleep(100);
  }
#endif // SINEN_PLATFORM_EMSCRIPTEN
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

  AdapterRequestState adapterState{};
  WGPURequestAdapterOptions adapterOptions{};
  adapterOptions.backendType = WGPUBackendType_Undefined;
  WGPURequestAdapterCallbackInfo adapterCallbackInfo{};
  adapterCallbackInfo.callback = &onAdapterRequest;
  adapterCallbackInfo.userdata1 = &adapterState;

  auto adapterFuture = wgpuInstanceRequestAdapter(instance, &adapterOptions,
                                                  adapterCallbackInfo);
  waitForRequest();
  if (!adapterState.adapter) {
    Log::error("Failed to request WebGPU adapter");
    wgpuInstanceRelease(instance);
    return nullptr;
  }

  DeviceRequestState deviceState{};
  WGPUDeviceDescriptor deviceDesc{};
  deviceDesc.label = toWgpuStringView("sinen-webgpu-device");
  deviceDesc.requiredFeatureCount = 0;
  deviceDesc.requiredFeatures = nullptr;
  deviceDesc.requiredLimits = nullptr;
  deviceDesc.defaultQueue.label = toWgpuStringView("sinen-webgpu-queue");
  deviceDesc.defaultQueue.nextInChain = nullptr;
  deviceDesc.deviceLostCallbackInfo = {};
  deviceDesc.deviceLostCallbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
  deviceDesc.deviceLostCallbackInfo.callback = nullptr;
  deviceDesc.uncapturedErrorCallbackInfo = {};

  WGPURequestDeviceCallbackInfo deviceCallbackInfo{};
  deviceCallbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
  deviceCallbackInfo.callback = &onDeviceRequest;
  deviceCallbackInfo.userdata1 = &deviceState;
  deviceCallbackInfo.userdata2 = nullptr;

  auto deviceFuture = wgpuAdapterRequestDevice(adapterState.adapter,
                                               &deviceDesc, deviceCallbackInfo);
  waitForRequest();
  if (!deviceState.device) {
    Log::error("Failed to request WebGPU device");
    wgpuAdapterRelease(adapterState.adapter);
    wgpuInstanceRelease(instance);
    return nullptr;
  }

  auto queue = wgpuDeviceGetQueue(deviceState.device);
  if (!queue) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get WebGPU queue");
    wgpuDeviceRelease(deviceState.device);
    wgpuAdapterRelease(adapterState.adapter);
    wgpuInstanceRelease(instance);
    return nullptr;
  }

  return makePtr<Device>(createInfo.allocator, createInfo, instance,
                         adapterState.adapter, deviceState.device, queue);
}
} // namespace sinen::gpu::webgpu
