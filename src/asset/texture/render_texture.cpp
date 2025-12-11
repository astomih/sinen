#include "../../graphics/graphics_system.hpp"
#include <asset/texture/render_texture.hpp>

namespace sinen {

RenderTexture::RenderTexture() : texture(nullptr) {}

void RenderTexture::create(int width, int height) {
  auto allocator = GraphicsSystem::getAllocator();
  auto device = GraphicsSystem::getDevice();
  rhi::Texture::CreateInfo info{};
  info.allocator = allocator;
  info.width = width;
  info.height = height;
  info.layerCountOrDepth = 1;
  info.format = GraphicsSystem::getDevice()->GetSwapchainFormat();
  info.usage = rhi::TextureUsage::ColorTarget;
  info.numLevels = 1;
  info.sampleCount = rhi::SampleCount::x1;
  info.type = rhi::TextureType::Texture2D;
  texture = device->CreateTexture(info);
  info.format = rhi::TextureFormat::D32_FLOAT_S8_UINT;
  info.usage = rhi::TextureUsage::DepthStencilTarget;
  depthStencil = device->CreateTexture(info);

  this->width = width;
  this->height = height;
}

rhi::Ptr<rhi::Texture> RenderTexture::getTexture() const { return texture; }

rhi::Ptr<rhi::Texture> RenderTexture::getDepthStencil() const {
  return depthStencil;
}
} // namespace sinen