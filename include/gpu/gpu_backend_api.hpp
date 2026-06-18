#ifndef SINEN_GPU_BACKEND_API_HPP
#define SINEN_GPU_BACKEND_API_HPP

#include <core/def/macro.hpp>

namespace sinen {

enum class GPUBackendAPI : int {
  Vulkan = 0,
#ifdef SINEN_PLATFORM_WINDOWS
  D3D12,
#endif
  WebGPU,
  SDLGPU,
  COUNT
};

} // namespace sinen

#endif
