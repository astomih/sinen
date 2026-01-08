#include <asset/texture/render_texture.hpp>
#include <core/allocator/global_allocator.hpp>
#include <graphics/graphics.hpp>

namespace sinen {

RenderTexture::RenderTexture() : texture(nullptr) {}

void RenderTexture::create(int width, int height) {
  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
  rhi::Texture::CreateInfo info{};
  info.allocator = allocator;
  info.width = width;
  info.height = height;
  info.layerCountOrDepth = 1;
  info.format = Graphics::getDevice()->getSwapchainFormat();
  info.usage = rhi::TextureUsage::ColorTarget;
  info.numLevels = 1;
  info.sampleCount = rhi::SampleCount::x1;
  info.type = rhi::TextureType::Texture2D;
  texture = device->createTexture(info);
  info.format = rhi::TextureFormat::D32_FLOAT_S8_UINT;
  info.usage = rhi::TextureUsage::DepthStencilTarget;
  depthStencil = device->createTexture(info);

  this->width = width;
  this->height = height;
}

Ptr<rhi::Texture> RenderTexture::getTexture() const { return texture; }

Ptr<rhi::Texture> RenderTexture::getDepthStencil() const {
  return depthStencil;
}
} // namespace sinen