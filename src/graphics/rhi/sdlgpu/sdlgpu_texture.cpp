#include "sdlgpu_texture.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_device.hpp"

#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
Texture::~Texture() {
  if (!isSwapchainTexture)
    SDL_ReleaseGPUTexture(device->getNative(), texture);
}
} // namespace sinen::rhi::sdlgpu
#endif // EMSCRIPTEN
