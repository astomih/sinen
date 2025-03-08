#include "../render/px_renderer.hpp"
#include "../render/render_system.hpp"
#include <texture/depth_stencil_texture.hpp>

namespace sinen {

DepthStencilTexture::DepthStencilTexture() {}
void DepthStencilTexture::create(int width, int height) {
  auto allocator = RendererImpl::GetPxRenderer()->GetAllocator();
  auto device = RendererImpl::GetPxRenderer()->GetDevice();
  px::Texture::CreateInfo info{};
  info.allocator = allocator;
  info.width = width;
  info.height = height;
  info.layerCountOrDepth = 1;
  info.format = px::TextureFormat::D32_FLOAT_S8_UINT;
  info.usage = px::TextureUsage::DepthStencilTarget;
  info.numLevels = 1;
  info.sampleCount = px::SampleCount::x1;
  info.type = px::TextureType::Texture2D;
  texture = device->CreateTexture(info);
}
px::Ptr<px::Texture> DepthStencilTexture::get_texture() const {
  return texture;
}
} // namespace sinen