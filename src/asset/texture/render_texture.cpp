#include "../../graphics/graphics_system.hpp"
#include <asset/texture/render_texture.hpp>

namespace sinen {

RenderTexture::RenderTexture() : texture(nullptr) {}

void RenderTexture::create(int width, int height) {
  auto allocator = GraphicsSystem::getAllocator();
  auto device = GraphicsSystem::getDevice();
  px::Texture::CreateInfo info{};
  info.allocator = allocator;
  info.width = width;
  info.height = height;
  info.layerCountOrDepth = 1;
  info.format = GraphicsSystem::getDevice()->GetSwapchainFormat();
  info.usage = px::TextureUsage::ColorTarget;
  info.numLevels = 1;
  info.sampleCount = px::SampleCount::x1;
  info.type = px::TextureType::Texture2D;
  texture = device->CreateTexture(info);
  info.format = px::TextureFormat::D32_FLOAT_S8_UINT;
  info.usage = px::TextureUsage::DepthStencilTarget;
  depthStencil = device->CreateTexture(info);

  this->width = width;
  this->height = height;
}

px::Ptr<px::Texture> RenderTexture::getTexture() const { return texture; }

px::Ptr<px::Texture> RenderTexture::getDepthStencil() const {
  return depthStencil;
}
} // namespace sinen