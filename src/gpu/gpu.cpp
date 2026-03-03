#include "core/data/ptr.hpp"
#include <SDL3/SDL.h>
#include <core/def/macro.hpp>

#ifdef SINEN_PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#endif
#include <gpu/gpu.hpp>

#include "sdlgpu/sdlgpu_backend.hpp"
#include "vulkan/vulkan_backend.hpp"
#ifndef SINEN_PLATFORM_ANDROID
#include "webgpu/webgpu_backend.hpp"
#endif

#include <SDL3/SDL.h>

namespace sinen::gpu {
Ptr<Backend> RHI::createBackend(Allocator *allocator,
                                const GPUBackendAPI &api) {
#ifndef SINEN_PLATFORM_EMSCRIPTEN
  switch (api) {
  case GPUBackendAPI::Vulkan: {
    return makePtr<vulkan::Backend>(allocator);
  }
#ifdef SINEN_PLATFORM_WINDOWS
  case GPUBackendAPI::D3D12U: {
    // TODO
    return nullptr;
  }
#endif
#ifndef SINEN_PLATFORM_ANDROID
  case GPUBackendAPI::WebGPU: {
    return makePtr<webgpu::Backend>(allocator);
  }
#endif
  case GPUBackendAPI::SDLGPU: {
    return makePtr<sdlgpu::Backend>(allocator);
  }
  default:
    return nullptr;
  }
#endif
  return nullptr;
}
} // namespace sinen::gpu
