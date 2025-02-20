#ifndef SINEN_TEXTURE_TEXTURE_CONTAINER_HPP
#define SINEN_TEXTURE_TEXTURE_CONTAINER_HPP
#include "utility/handle_t.hpp"
#include <paranoixa/paranoixa.hpp>
#include <unordered_map>

namespace sinen {
class TextureContainer {
public:
  // operator
  static px::Ptr<px::Texture> &at(const HandleT &handle) {
    return hashMap[handle];
  }
  static std::unordered_map<HandleT, px::Ptr<px::Texture>> hashMap;
};
px::Ptr<px::Texture> CreateNativeTexture(const HandleT &handle);
void UpdateNativeTexture(px::Ptr<px::Texture> texture, const HandleT &handle);
} // namespace sinen
#endif // SINEN_TEXTURE_TEXTURE_CONTAINER_HPP