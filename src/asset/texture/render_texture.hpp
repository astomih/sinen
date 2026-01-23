#ifndef SINEN_RENDER_TEXTURE_HPP
#define SINEN_RENDER_TEXTURE_HPP
#include <graphics/rhi/rhi.hpp>
namespace sinen {
class RenderTexture {
public:
  RenderTexture();

  static constexpr const char *metaTableName() { return "sn.RenderTexture"; }

  void create(int width, int height);

  int width;
  int height;

  Ptr<rhi::Texture> getTexture() const;
  Ptr<rhi::Texture> getDepthStencil() const;

private:
  Ptr<rhi::Texture> texture;
  Ptr<rhi::Texture> depthStencil;
};
} // namespace sinen
#endif // SINEN_RENDER_TEXTURE_HPP
