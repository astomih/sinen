#ifndef SINEN_TEXTURE_TEXTURE_CONTAINER_HPP
#define SINEN_TEXTURE_TEXTURE_CONTAINER_HPP
#include "utility/handle_t.hpp"
#include <memory>
#include <paranoixa/paranoixa.hpp>
#include <unordered_map>

namespace sinen {
struct TextureData {
  ~TextureData();
  HandleT handle;
  px::Ptr<px::Texture> texture;
};
inline std::shared_ptr<TextureData> GetTexData(std::shared_ptr<void> tex) {
  return std::static_pointer_cast<TextureData>(tex);
}
px::Ptr<px::Texture> CreateNativeTexture(const HandleT &handle);
void UpdateNativeTexture(px::Ptr<px::Texture> texture, const HandleT &handle);
} // namespace sinen
#endif // SINEN_TEXTURE_TEXTURE_CONTAINER_HPP