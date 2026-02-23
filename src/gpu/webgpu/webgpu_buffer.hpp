#ifndef EMSCRIPTEN
#ifndef SINEN_WEBGPU_BUFFER_HPP
#define SINEN_WEBGPU_BUFFER_HPP

#include <core/data/ptr.hpp>
#include <gpu/gpu_buffer.hpp>
#include <webgpu/webgpu.h>

namespace sinen::gpu::webgpu {
class Device;

class Buffer : public gpu::Buffer {
public:
  Buffer(const CreateInfo &createInfo, const Ptr<Device> &device,
         WGPUBuffer buffer)
      : gpu::Buffer(createInfo), device(device), buffer(buffer) {}
  ~Buffer() override;

  WGPUBuffer getNative() const { return buffer; }

private:
  Ptr<Device> device;
  WGPUBuffer buffer;
};
} // namespace sinen::gpu::webgpu

#endif // SINEN_WEBGPU_BUFFER_HPP
#endif // EMSCRIPTEN
