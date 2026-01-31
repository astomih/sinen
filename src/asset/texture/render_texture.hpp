#ifndef SINEN_RENDER_TEXTURE_HPP
#define SINEN_RENDER_TEXTURE_HPP
#include <gpu/gpu.hpp>
namespace sinen {
class RenderTexture {
public:
  RenderTexture();

  static constexpr const char *metaTableName() { return "sn.RenderTexture"; }

  void create(int width, int height);

  int width;
  int height;

  Ptr<gpu::Texture> getTexture() const;
  Ptr<gpu::Texture> getDepthStencil() const;

private:
  Ptr<gpu::Texture> texture;
  Ptr<gpu::Texture> depthStencil;
};
} // namespace sinen
#endif // SINEN_RENDER_TEXTURE_HPP
