#include "core/data/ptr.hpp"
#include <SDL3/SDL.h>
#include <core/def/macro.hpp>

#ifdef SINEN_PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#endif
#include <gpu/gpu.hpp>

#include "sdlgpu/sdlgpu_backend.hpp"
#include "vulkan/vulkan_backend.hpp"

#include <SDL3/SDL.h>

namespace sinen::gpu {
Ptr<Backend> RHI::createBackend(Allocator *allocator, const GraphicsAPI &api) {
#ifndef SINEN_PLATFORM_EMSCRIPTEN
  switch (api) {
  case GraphicsAPI::Vulkan: {
    return makePtr<vulkan::Backend>(allocator);
  }
#ifdef SINEN_PLATFORM_WINDOWS
  case GraphicsAPI::D3D12U: {
    // TODO
    return nullptr;
  }
#endif
  case GraphicsAPI::WebGPU: {
    // TODO
    return nullptr;
  }
  case GraphicsAPI::SDLGPU: {
    return makePtr<sdlgpu::Backend>(allocator);
  }
  default:
    return nullptr;
  }
#endif
  return nullptr;
}
} // namespace sinen::gpu
