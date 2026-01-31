#include "core/data/ptr.hpp"
#include <SDL3/SDL.h>
#include <core/def/macro.hpp>

#ifdef SINEN_PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#endif
#include <gpu/gpu.hpp>

#include "d3d12u/d3d12u_renderer.hpp"
#include "sdlgpu/sdlgpu_backend.hpp"
#include "vulkan/vulkan_renderer.hpp"
#include "webgpu/webgpu_renderer.hpp"

#include <SDL3/SDL.h>

namespace sinen::gpu {
Ptr<Backend> RHI::createBackend(Allocator *allocator, const GraphicsAPI &api) {
#ifndef SINEN_PLATFORM_EMSCRIPTEN
  switch (api) {
  case GraphicsAPI::Vulkan: {
    // TODO
  }
#ifdef SINEN_PLATFORM_WINDOWS
  case GraphicsAPI::D3D12U: {
    // TODO
  }
#endif
  case GraphicsAPI::WebGPU: {
    // TODO
  }
  case GraphicsAPI::SDLGPU: {
    Ptr<Backend> p = makePtr<sdlgpu::Backend>(allocator);
    return p;
  }
  default:
    return nullptr;
  }
#endif
  return nullptr;
}
} // namespace sinen::gpu
