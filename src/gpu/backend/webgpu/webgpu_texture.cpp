#include "webgpu_texture.hpp"

namespace sinen::gpu::webgpu {
Texture::~Texture() {
  if (textureView) {
    wgpuTextureViewRelease(textureView);
    textureView = nullptr;
  }
  if (texture) {
    if (!isSwapchainTexture) {
      wgpuTextureDestroy(texture);
    }
    wgpuTextureRelease(texture);
    texture = nullptr;
  }
}
} // namespace sinen::gpu::webgpu
