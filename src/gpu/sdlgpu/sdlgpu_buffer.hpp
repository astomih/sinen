#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_BUFFER_HPP
#define SINEN_SDLGPU_BUFFER_HPP

#include <core/data/ptr.hpp>
#include <gpu/gpu_buffer.hpp>
#include <SDL3/SDL_gpu.h>

namespace sinen::gpu::sdlgpu {
class Device;

class Buffer : public gpu::Buffer {
public:
  Buffer(const CreateInfo &createInfo, const Ptr<Device> &device,
         SDL_GPUBuffer *buffer)
      : gpu::Buffer(createInfo), device(device), buffer(buffer) {}
  ~Buffer() override;

  SDL_GPUBuffer *getNative() { return buffer; }

private:
  Ptr<Device> device;
  SDL_GPUBuffer *buffer;
};
} // namespace sinen::gpu::sdlgpu

#endif // SINEN_SDLGPU_BUFFER_HPP
#endif // EMSCRIPTEN
