#ifndef SINEN_GPU_BACKEND_API_HPP
#define SINEN_GPU_BACKEND_API_HPP

#include <core/def/macro.hpp>

namespace sinen {

enum class GPUBackendAPI {
  Vulkan,
#ifdef SINEN_PLATFORM_WINDOWS
  D3D12U,
#endif
  WebGPU,
  SDLGPU,

};

} // namespace sinen

#endif
