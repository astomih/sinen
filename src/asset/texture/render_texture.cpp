#include <asset/texture/render_texture.hpp>
#include <core/allocator/global_allocator.hpp>
#include <graphics/graphics.hpp>

namespace sinen {

RenderTexture::RenderTexture() : texture(nullptr) {}

void RenderTexture::create(int width, int height) {
  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
  gpu::Texture::CreateInfo info{};
  info.allocator = allocator;
  info.width = width;
  info.height = height;
  info.layerCountOrDepth = 1;
  info.format = Graphics::getDevice()->getSwapchainFormat();
  info.usage = gpu::TextureUsage::ColorTarget;
  info.numLevels = 1;
  info.sampleCount = gpu::SampleCount::x1;
  info.type = gpu::TextureType::Texture2D;
  texture = device->createTexture(info);
  info.format = gpu::TextureFormat::D32_FLOAT_S8_UINT;
  info.usage = gpu::TextureUsage::DepthStencilTarget;
  depthStencil = device->createTexture(info);

  this->width = width;
  this->height = height;
}

Ptr<gpu::Texture> RenderTexture::getTexture() const { return texture; }

Ptr<gpu::Texture> RenderTexture::getDepthStencil() const {
  return depthStencil;
}
} // namespace sinen