#ifndef EMSCRIPTEN
#ifndef SINEN_VULKAN_BACKEND_HPP
#define SINEN_VULKAN_BACKEND_HPP

#include <gpu/gpu.hpp>

namespace sinen::gpu::vulkan {
class Backend : public gpu::Backend {
public:
  Ptr<gpu::Device>
  createDevice(const gpu::Device::CreateInfo &createInfo) override;
};
} // namespace sinen::gpu::vulkan

#endif // SINEN_VULKAN_BACKEND_HPP
#endif // EMSCRIPTEN

