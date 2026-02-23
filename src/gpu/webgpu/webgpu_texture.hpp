#ifndef EMSCRIPTEN
#ifndef SINEN_WEBGPU_TEXTURE_HPP
#define SINEN_WEBGPU_TEXTURE_HPP

#include <core/data/ptr.hpp>
#include <gpu/gpu_texture.hpp>
#include <webgpu/webgpu.h>

namespace sinen::gpu::webgpu {
class Device;

class Texture : public gpu::Texture {
public:
  Texture(const CreateInfo &createInfo, const Ptr<Device> &device,
          WGPUTexture texture, WGPUTextureView textureView,
          bool isSwapchainTexture = false)
      : gpu::Texture(createInfo), device(device), texture(texture),
        textureView(textureView), isSwapchainTexture(isSwapchainTexture) {}
  ~Texture() override;

  WGPUTexture getNative() const { return texture; }
  WGPUTextureView getView() const { return textureView; }
  bool isSwapchain() const { return isSwapchainTexture; }

private:
  Ptr<Device> device;
  WGPUTexture texture;
  WGPUTextureView textureView;
  bool isSwapchainTexture;
};
} // namespace sinen::gpu::webgpu

#endif // SINEN_WEBGPU_TEXTURE_HPP
#endif // EMSCRIPTEN
