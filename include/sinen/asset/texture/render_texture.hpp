#ifndef SINEN_RENDER_TEXTURE_HPP
#define SINEN_RENDER_TEXTURE_HPP
#include <graphics/rhi/rhi.hpp>
namespace sinen {
class RenderTexture {
public:
  RenderTexture();

  void create(int width, int height);

  int width;
  int height;

  rhi::Ptr<rhi::Texture> getTexture() const;
  rhi::Ptr<rhi::Texture> getDepthStencil() const;

private:
  rhi::Ptr<rhi::Texture> texture;
  rhi::Ptr<rhi::Texture> depthStencil;
};
} // namespace sinen
#endif // SINEN_RENDER_TEXTURE_HPP