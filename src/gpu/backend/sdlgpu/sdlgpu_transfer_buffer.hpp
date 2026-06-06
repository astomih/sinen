#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_TRANSFER_BUFFER_HPP
#define SINEN_SDLGPU_TRANSFER_BUFFER_HPP

#include <core/data/ptr.hpp>
#include <gpu/gpu_transfer_buffer.hpp>
#include <SDL3/SDL_gpu.h>

namespace sinen::gpu::sdlgpu {
class Device;

class TransferBuffer : public gpu::TransferBuffer {
public:
  TransferBuffer(const CreateInfo &createInfo, const Ptr<Device> &device,
                 SDL_GPUTransferBuffer *transferBuffer)
      : gpu::TransferBuffer(createInfo), device(device),
        transferBuffer(transferBuffer) {}
  ~TransferBuffer() override;

  SDL_GPUTransferBuffer *getNative() { return transferBuffer; }

  void *map(bool cycle) override;
  void unmap() override;

private:
  Ptr<Device> device;
  SDL_GPUTransferBuffer *transferBuffer;
};
} // namespace sinen::gpu::sdlgpu

#endif // SINEN_SDLGPU_TRANSFER_BUFFER_HPP
#endif // EMSCRIPTEN
