#include "core/data/ptr.hpp"
#include <SDL3/SDL.h>
#include <memory_resource>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif // __EMSCRIPTEN__
#include <graphics/rhi/rhi.hpp>

#include <core/allocator/std_allocator.hpp>
#include <core/allocator/tlsf_allocator.hpp>

#include "d3d12u/d3d12u_renderer.hpp"
#include "sdlgpu/sdlgpu_backend.hpp"
#include "vulkan/vulkan_renderer.hpp"
#include "webgpu/webgpu_renderer.hpp"

#include <SDL3/SDL.h>

#include <fstream>
#include <iostream>
namespace sinen::rhi {
Ptr<Backend> RHI::createBackend(Allocator *allocator, const GraphicsAPI &api) {
#ifndef SINEN_PLATFORM_EMSCRIPTEN
  switch (api) {
  case GraphicsAPI::Vulkan: {
    // TODO
  }
#ifdef _WIN32
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
} // namespace sinen::rhi