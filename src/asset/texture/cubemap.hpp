#ifndef SINEN_CUBEMAP_HPP
#define SINEN_CUBEMAP_HPP
#include <string_view>

#include <graphics/rhi/rhi.hpp>

namespace sinen {
class Cubemap {
public:
  bool load(std::string_view path);

  rhi::Ptr<rhi::Texture> getNativeCubemap() const { return nativeCubemap; }

private:
  rhi::Ptr<rhi::Texture> nativeCubemap;
};
} // namespace sinen

#endif // SINEN_CUBEMAP_HPP
