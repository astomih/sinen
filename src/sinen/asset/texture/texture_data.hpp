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
inline std::shared_ptr<TextureData> GetTexData(std::shared_ptr<void> tex) {
  return std::static_pointer_cast<TextureData>(tex);
}
px::Ptr<px::Texture> CreateNativeTexture(SDL_Surface *pSurface);
void UpdateNativeTexture(px::Ptr<px::Texture> texture, SDL_Surface *pSurface);
} // namespace sinen
#endif // SINEN_TEXTURE_TEXTURE_CONTAINER_HPP