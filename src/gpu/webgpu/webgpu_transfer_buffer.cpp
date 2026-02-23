#include "webgpu_transfer_buffer.hpp"

#ifndef EMSCRIPTEN
#include "webgpu_device.hpp"
#include "webgpu_convert.hpp"
#include <SDL3/SDL.h>

namespace sinen::gpu::webgpu {
TransferBuffer::~TransferBuffer() {
  if (transferBuffer) {
    wgpuBufferDestroy(transferBuffer);
    wgpuBufferRelease(transferBuffer);
    transferBuffer = nullptr;
  }
}

void TransferBuffer::onMapComplete(WGPUMapAsyncStatus status,
                                   WGPUStringView message, void *userdata1,
                                   void *userdata2) {
  (void)message;
  (void)userdata2;
  auto *state = static_cast<MapState *>(userdata1);
  state->done = true;
  state->success = (status == WGPUMapAsyncStatus_Success);
}

void *TransferBuffer::map(bool cycle) {
  (void)cycle;
  if (getCreateInfo().usage == TransferBufferUsage::Upload) {
    return uploadData.empty() ? nullptr : uploadData.data();
  }

  if (!transferBuffer) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "TransferBuffer map failed: native download buffer is null");
    return nullptr;
  }

  MapState state{};
  WGPUBufferMapCallbackInfo callbackInfo{};
  callbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
  callbackInfo.callback = &TransferBuffer::onMapComplete;
  callbackInfo.userdata1 = &state;
  callbackInfo.userdata2 = nullptr;

  const auto future =
      wgpuBufferMapAsync(transferBuffer, convert::MapModeFrom(getCreateInfo().usage),
                         0, WGPU_WHOLE_MAP_SIZE, callbackInfo);
  if (!device->waitForFuture(future) || !state.done || !state.success) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "TransferBuffer map failed: map async did not complete");
    return nullptr;
  }

  if (getCreateInfo().usage == TransferBufferUsage::Download) {
    auto p = wgpuBufferGetConstMappedRange(transferBuffer, 0, WGPU_WHOLE_MAP_SIZE);
    return const_cast<void *>(p);
  }
  return wgpuBufferGetMappedRange(transferBuffer, 0, WGPU_WHOLE_MAP_SIZE);
}

void TransferBuffer::unmap() {
  if (getCreateInfo().usage == TransferBufferUsage::Download) {
    if (transferBuffer) {
      wgpuBufferUnmap(transferBuffer);
    }
  }
}
} // namespace sinen::gpu::webgpu

#endif // EMSCRIPTEN
