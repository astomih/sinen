#include "d3d12_backend.hpp"

#ifdef SINEN_PLATFORM_WINDOWS

#include "d3d12_device.hpp"

namespace sinen::gpu::d3d12 {
Ptr<gpu::Device> Backend::createDevice(const gpu::Device::CreateInfo &createInfo) {
  auto device = makePtr<Device>(createInfo.allocator, createInfo);
  if (!device->isValid()) {
    return nullptr;
  }
  return device;
}
} // namespace sinen::gpu::d3d12

#endif // SINEN_PLATFORM_WINDOWS
