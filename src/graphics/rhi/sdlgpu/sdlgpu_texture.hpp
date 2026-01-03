#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_TEXTURE_HPP
#define SINEN_SDLGPU_TEXTURE_HPP

#include <core/data/ptr.hpp>
#include <graphics/rhi/rhi_texture.hpp>
#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
class Device;

class Texture : public rhi::Texture {
public:
  Texture(const CreateInfo &createInfo, const Ptr<Device> &device,
          SDL_GPUTexture *texture, bool isSwapchainTexture = false)
      : rhi::Texture(createInfo), device(device), texture(texture),
        isSwapchainTexture(isSwapchainTexture) {}
  ~Texture() override;

  SDL_GPUTexture *getNative() const { return texture; }

private:
  Ptr<Device> device;
  SDL_GPUTexture *texture;
  bool isSwapchainTexture;
};
} // namespace sinen::rhi::sdlgpu

#endif // SINEN_SDLGPU_TEXTURE_HPP
#endif // EMSCRIPTEN
