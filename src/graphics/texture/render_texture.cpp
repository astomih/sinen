#include <core/allocator/global_allocator.hpp>
#include <graphics/graphics.hpp>
#include <graphics/texture/render_texture.hpp>

namespace sinen {

RenderTexture::RenderTexture() : texture(nullptr), colorTarget(nullptr) {}

static gpu::SampleCount sampleCountFromValue(UInt32 value) {
  switch (value) {
  case 2:
    return gpu::SampleCount::x2;
  case 4:
    return gpu::SampleCount::x4;
  case 8:
    return gpu::SampleCount::x8;
  case 1:
  default:
    return gpu::SampleCount::x1;
  }
}

void RenderTexture::create(int width, int height) {
  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
  const auto sampleCount = sampleCountFromValue(Graphics::getMSAASampleCount());
  const bool useMSAA = sampleCount != gpu::SampleCount::x1;

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
  info.debugName =
      useMSAA ? "RenderTexture resolve color" : "RenderTexture color";
  texture = device->createTexture(info);
  if (useMSAA) {
    info.sampleCount = sampleCount;
    info.debugName = "RenderTexture MSAA color";
    colorTarget = device->createTexture(info);
  } else {
    colorTarget = texture;
  }
  info.format = gpu::TextureFormat::D32_FLOAT_S8_UINT;
  info.usage = gpu::TextureUsage::DepthStencilTarget;
  info.sampleCount = sampleCount;
  info.debugName = "RenderTexture depth";
  depthStencil = device->createTexture(info);

  this->width = width;
  this->height = height;
}

Ptr<gpu::Texture> RenderTexture::getTexture() const { return texture; }

Ptr<gpu::Texture> RenderTexture::getColorTarget() const { return colorTarget; }

Ptr<gpu::Texture> RenderTexture::getDepthStencil() const {
  return depthStencil;
}

} // namespace sinen
