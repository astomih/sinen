#ifndef SINEN_TEXTURE_TEXTURE_CONTAINER_HPP
#define SINEN_TEXTURE_TEXTURE_CONTAINER_HPP

#include <SDL3/SDL.h>
#include <memory>
#include <paranoixa/paranoixa.hpp>
#include <unordered_map>

namespace sinen {
struct TextureData {
  ~TextureData();
  SDL_Surface *pSurface;
  px::Ptr<px::Texture> texture;
};
inline std::shared_ptr<TextureData>
getTextureRawData(std::shared_ptr<void> tex) {
  return std::static_pointer_cast<TextureData>(tex);
}
px::Ptr<px::Texture> createNativeTexture(void *pPixels,
                                         px::TextureFormat textureFormat,
                                         uint32_t width, uint32_t height);
px::Ptr<px::Texture> createNativeTexture(SDL_Surface *pSurface);
void UpdateNativeTexture(px::Ptr<px::Texture> texture, void *pPixels);
void UpdateNativeTexture(px::Ptr<px::Texture> texture, SDL_Surface *pSurface);
} // namespace sinen
#endif // SINEN_TEXTURE_TEXTURE_CONTAINER_HPP