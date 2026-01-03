#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_BACKEND_HPP
#define SINEN_SDLGPU_BACKEND_HPP

#include <graphics/rhi/rhi.hpp>

namespace sinen::rhi::sdlgpu {
class Backend : public rhi::Backend {
public:
  Ptr<rhi::Device>
  createDevice(const rhi::Device::CreateInfo &createInfo) override;
};
} // namespace sinen::rhi::sdlgpu

#endif // SINEN_SDLGPU_BACKEND_HPP
#endif // EMSCRIPTEN
