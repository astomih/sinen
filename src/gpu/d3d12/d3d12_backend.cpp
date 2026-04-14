#include "d3d12_backend.hpp"

#ifdef SINEN_PLATFORM_WINDOWS

#include "d3d12_device.hpp"

namespace sinen::gpu::d3d12 {
Ptr<gpu::Device> Backend::createDevice(const gpu::Device::CreateInfo &createInfo) {
  return makePtr<Device>(createInfo.allocator, createInfo);
}
} // namespace sinen::gpu::d3d12

#endif // SINEN_PLATFORM_WINDOWS
