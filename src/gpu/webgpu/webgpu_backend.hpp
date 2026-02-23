#ifndef SINEN_WEBGPU_BACKEND_HPP
#define SINEN_WEBGPU_BACKEND_HPP

#include <gpu/gpu.hpp>

namespace sinen::gpu::webgpu {
class Backend : public gpu::Backend {
public:
  Ptr<gpu::Device>
  createDevice(const gpu::Device::CreateInfo &createInfo) override;
};
} // namespace sinen::gpu::webgpu

#endif // SINEN_WEBGPU_BACKEND_HPP
