#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_BACKEND_HPP
#define SINEN_SDLGPU_BACKEND_HPP

#include <gpu/gpu.hpp>

namespace sinen::gpu::sdlgpu {
class Backend : public gpu::Backend {
public:
  Ptr<gpu::Device>
  createDevice(const gpu::Device::CreateInfo &createInfo) override;
};
} // namespace sinen::gpu::sdlgpu

#endif // SINEN_SDLGPU_BACKEND_HPP
#endif // EMSCRIPTEN
