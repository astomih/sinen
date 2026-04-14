#ifndef SINEN_D3D12_BACKEND_HPP
#define SINEN_D3D12_BACKEND_HPP

#include <core/def/macro.hpp>

#ifdef SINEN_PLATFORM_WINDOWS

#include <gpu/gpu.hpp>

namespace sinen::gpu::d3d12 {
class Backend : public gpu::Backend {
public:
  Ptr<gpu::Device>
  createDevice(const gpu::Device::CreateInfo &createInfo) override;
};
} // namespace sinen::gpu::d3d12

#endif // SINEN_PLATFORM_WINDOWS

#endif // SINEN_D3D12_BACKEND_HPP
