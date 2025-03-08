#ifndef SINEN_DEPTH_STENCIL_TEXTURE_HPP
#define SINEN_DEPTH_STENCIL_TEXTURE_HPP
#include <paranoixa/paranoixa.hpp>
namespace sinen {
class DepthStencilTexture {
public:
  DepthStencilTexture();

  void create(int width, int height);

  px::Ptr<px::Texture> get_texture() const;

private:
  px::Ptr<px::Texture> texture;
};
} // namespace sinen
#endif // SINEN_DEPTH_STENCIL_TEXTURE_HPP