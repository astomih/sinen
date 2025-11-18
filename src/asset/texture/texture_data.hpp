#ifndef SINEN_TEXTURE_TEXTURE_CONTAINER_HPP
#define SINEN_TEXTURE_TEXTURE_CONTAINER_HPP

#include <SDL3/SDL.h>
#include <memory>
#include <paranoixa/paranoixa.hpp>
#include <unordered_map>

namespace sinen {
px::Ptr<px::Texture> createNativeTexture(void *pPixels,
                                         px::TextureFormat textureFormat,
                                         uint32_t width, uint32_t height);
void UpdateNativeTexture(px::Ptr<px::Texture> texture, void *pPixels);
} // namespace sinen
#endif // SINEN_TEXTURE_TEXTURE_CONTAINER_HPP