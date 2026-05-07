#include "core/data/ptr.hpp"
#include <SDL3/SDL.h>
#include <core/def/macro.hpp>

#ifdef SINEN_PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#endif
#include <gpu/gpu.hpp>

#ifdef SINEN_ENABLE_WEBGPU
#include "webgpu/webgpu_backend.hpp"
#endif
#ifndef SINEN_PLATFORM_EMSCRIPTEN
#ifdef SINEN_PLATFORM_WINDOWS
#include "d3d12/d3d12_backend.hpp"
#endif
#include "sdlgpu/sdlgpu_backend.hpp"
#include "vulkan/vulkan_backend.hpp"
#endif

#include <SDL3/SDL.h>

namespace sinen::gpu {
Ptr<Backend> RHI::createBackend(Allocator *allocator,
                                const GPUBackendAPI &api) {
#ifdef SINEN_PLATFORM_EMSCRIPTEN
#ifdef SINEN_ENABLE_WEBGPU
  if (api == GPUBackendAPI::WebGPU) {
    return makePtr<webgpu::Backend>(allocator);
  }
#endif
  return nullptr;
#else
  switch (api) {
  case GPUBackendAPI::Vulkan: {
    return makePtr<vulkan::Backend>(allocator);
  }
#ifdef SINEN_PLATFORM_WINDOWS
  case GPUBackendAPI::D3D12: {
    return makePtr<d3d12::Backend>(allocator);
  }
#endif
#ifdef SINEN_ENABLE_WEBGPU
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
}
} // namespace sinen::gpu
