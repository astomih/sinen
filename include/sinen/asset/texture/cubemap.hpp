#ifndef SINEN_CUBEMAP_HPP
#define SINEN_CUBEMAP_HPP
#include <string_view>

#include <graphics/paranoixa/paranoixa.hpp>

namespace sinen {
class Cubemap {
public:
  bool load(std::string_view path);

  px::Ptr<px::Texture> getNativeCubemap() const { return nativeCubemap; }

private:
  px::Ptr<px::Texture> nativeCubemap;
};
} // namespace sinen

#endif // SINEN_CUBEMAP_HPP
