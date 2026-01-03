#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_BUFFER_HPP
#define SINEN_SDLGPU_BUFFER_HPP

#include <core/data/ptr.hpp>
#include <graphics/rhi/rhi_buffer.hpp>
#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
class Device;

class Buffer : public rhi::Buffer {
public:
  Buffer(const CreateInfo &createInfo, const Ptr<Device> &device,
         SDL_GPUBuffer *buffer)
      : rhi::Buffer(createInfo), device(device), buffer(buffer) {}
  ~Buffer() override;

  SDL_GPUBuffer *getNative() { return buffer; }

private:
  Ptr<Device> device;
  SDL_GPUBuffer *buffer;
};
} // namespace sinen::rhi::sdlgpu

#endif // SINEN_SDLGPU_BUFFER_HPP
#endif // EMSCRIPTEN
