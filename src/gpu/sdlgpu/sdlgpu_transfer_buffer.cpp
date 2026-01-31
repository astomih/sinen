#include "sdlgpu_transfer_buffer.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_device.hpp"

#include <SDL3/SDL_gpu.h>

namespace sinen::gpu::sdlgpu {
TransferBuffer::~TransferBuffer() {
  SDL_ReleaseGPUTransferBuffer(device->getNative(), transferBuffer);
}

void *TransferBuffer::map(bool cycle) {
  return SDL_MapGPUTransferBuffer(device->getNative(), this->transferBuffer,
                                  cycle);
}

void TransferBuffer::unmap() {
  SDL_UnmapGPUTransferBuffer(device->getNative(), this->transferBuffer);
}
} // namespace sinen::gpu::sdlgpu
#endif // EMSCRIPTEN
