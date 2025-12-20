#ifndef SINEN_TEXTURE_TEXTURE_CONTAINER_HPP
#define SINEN_TEXTURE_TEXTURE_CONTAINER_HPP

#include <SDL3/SDL.h>
#include <graphics/rhi/rhi.hpp>
#include <memory>
#include <unordered_map>

namespace sinen {
Ptr<rhi::Texture> createNativeTexture(void *pPixels,
                                      rhi::TextureFormat textureFormat,
                                      uint32_t width, uint32_t height);
void updateNativeTexture(Ptr<rhi::Texture> texture, void *pPixels);
} // namespace sinen
#endif // SINEN_TEXTURE_TEXTURE_CONTAINER_HPP