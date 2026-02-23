#include "webgpu_backend.hpp"

#ifndef EMSCRIPTEN
#include "webgpu_device.hpp"
#include <SDL3/SDL.h>
#include <thread>
#include <webgpu/webgpu.h>

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

bool waitForFuture(WGPUInstance instance, WGPUFuture future) {
  if (!instance || future.id == 0) {
    return false;
  }
  WGPUFutureWaitInfo waitInfo{};
  waitInfo.future = future;
  waitInfo.completed = false;
  while (true) {
    waitInfo.completed = false;
    const auto status = wgpuInstanceWaitAny(instance, 1, &waitInfo, 0);
    if (status == WGPUWaitStatus_Success && waitInfo.completed) {
      return true;
    }
    if (status == WGPUWaitStatus_TimedOut || status == WGPUWaitStatus_Success) {
      wgpuInstanceProcessEvents(instance);
      std::this_thread::yield();
      continue;
    }
    return false;
  }
}

void onAdapterRequest(WGPURequestAdapterStatus status, WGPUAdapter adapter,
                      WGPUStringView message, void *userdata1,
                      void *userdata2) {
  (void)message;
  (void)userdata2;
  auto *state = static_cast<AdapterRequestState *>(userdata1);
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

Ptr<gpu::Device> Backend::createDevice(const gpu::Device::CreateInfo &createInfo) {
  (void)createInfo.debugMode;
  WGPUInstanceDescriptor instanceDesc{};
  auto instance = wgpuCreateInstance(&instanceDesc);
  if (!instance) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to create WebGPU instance");
    return nullptr;
  }

  AdapterRequestState adapterState{};
  WGPURequestAdapterOptions adapterOptions{};
  adapterOptions.featureLevel = WGPUFeatureLevel_Core;
  adapterOptions.powerPreference = WGPUPowerPreference_HighPerformance;
  adapterOptions.forceFallbackAdapter = false;
  adapterOptions.backendType = WGPUBackendType_Undefined;
  adapterOptions.compatibleSurface = nullptr;

  WGPURequestAdapterCallbackInfo adapterCallbackInfo{};
  adapterCallbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
  adapterCallbackInfo.callback = &onAdapterRequest;
  adapterCallbackInfo.userdata1 = &adapterState;
  adapterCallbackInfo.userdata2 = nullptr;

  auto adapterFuture =
      wgpuInstanceRequestAdapter(instance, &adapterOptions, adapterCallbackInfo);
  if (!waitForFuture(instance, adapterFuture) || !adapterState.done ||
      !adapterState.success || !adapterState.adapter) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to request WebGPU adapter");
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

  auto deviceFuture =
      wgpuAdapterRequestDevice(adapterState.adapter, &deviceDesc, deviceCallbackInfo);
  if (!waitForFuture(instance, deviceFuture) || !deviceState.done ||
      !deviceState.success || !deviceState.device) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to request WebGPU device");
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
#endif // EMSCRIPTEN
