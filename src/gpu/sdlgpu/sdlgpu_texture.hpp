#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_TEXTURE_HPP
#define SINEN_SDLGPU_TEXTURE_HPP

#include <core/data/ptr.hpp>
#include <gpu/gpu_texture.hpp>
#include <SDL3/SDL_gpu.h>

namespace sinen::gpu::sdlgpu {
class Device;

class Texture : public gpu::Texture {
public:
  Texture(const CreateInfo &createInfo, const Ptr<Device> &device,
          SDL_GPUTexture *texture, bool isSwapchainTexture = false)
      : gpu::Texture(createInfo), device(device), texture(texture),
        isSwapchainTexture(isSwapchainTexture) {}
  ~Texture() override;

  SDL_GPUTexture *getNative() const { return texture; }

private:
  Ptr<Device> device;
  SDL_GPUTexture *texture;
  bool isSwapchainTexture;
};
} // namespace sinen::gpu::sdlgpu

#endif // SINEN_SDLGPU_TEXTURE_HPP
#endif // EMSCRIPTEN
