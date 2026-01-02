#ifndef SINEN_RHI_HPP
#define SINEN_RHI_HPP
#include <cassert>
#include <core/data/array.hpp>
#include <core/data/hashmap.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <core/def/macro.hpp>
#include <core/def/types.hpp>

#include "rhi_device.hpp"

namespace sinen::rhi {

enum class GraphicsAPI {
  Vulkan,
#ifdef SINEN_PLATFORM_WINDOWS
  D3D12U,
#endif
  WebGPU,
  SDLGPU,
};

class Backend {
public:
  Backend() = default;
  virtual ~Backend() = default;
  virtual Ptr<Device> createDevice(const Device::CreateInfo &createInfo) = 0;
};

class RHI {
public:
  static Ptr<Backend> createBackend(Allocator *allocator,
                                    const GraphicsAPI &api);
};
} // namespace sinen::rhi
#endif
