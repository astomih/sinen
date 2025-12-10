#ifndef SINEN_RENDER_TEXTURE_HPP
#define SINEN_RENDER_TEXTURE_HPP
#include <graphics/paranoixa/paranoixa.hpp>
namespace sinen {
class RenderTexture {
public:
  RenderTexture();

  void create(int width, int height);

  int width;
  int height;

  px::Ptr<px::Texture> getTexture() const;
  px::Ptr<px::Texture> getDepthStencil() const;

private:
  px::Ptr<px::Texture> texture;
  px::Ptr<px::Texture> depthStencil;
};
} // namespace sinen
#endif // SINEN_RENDER_TEXTURE_HPP