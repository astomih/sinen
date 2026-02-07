#ifndef EMSCRIPTEN
#include "vulkan_backend.hpp"
#include "vulkan_device.hpp"

namespace sinen::gpu::vulkan {
Ptr<gpu::Device>
Backend::createDevice(const gpu::Device::CreateInfo &createInfo) {
  return makePtr<Device>(createInfo.allocator, createInfo);
}
} // namespace sinen::gpu::vulkan
#endif // EMSCRIPTEN
