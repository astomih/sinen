#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <cassert>
#include <io/data_stream.hpp>
#include <logger/logger.hpp>
#include <memory>
#include <string_view>
#include <texture/texture.hpp>
#include <unordered_map>
#include <utility/handle_t.hpp>

namespace sinen {
struct SDLObjectCloser {
  void operator()(::SDL_Surface *surface);
  void operator()(::SDL_IOStream *rw);
};
void SDLObjectCloser::operator()(::SDL_Surface *surface) {
  if (surface) {
    SDL_DestroySurface(surface);
  }
}
void SDLObjectCloser::operator()(::SDL_IOStream *rw) { SDL_CloseIO(rw); }
SDL_Surface &get(HandleT handle) {
  SDL_Surface *surf = reinterpret_cast<SDL_Surface *>(handle);
  return *surf;
}
Texture::Texture() {
  this->handle = create();
  is_need_update = std::make_shared<bool>(false);
}
Texture::Texture(const Texture &other) {
  if (is_need_update.use_count() == 1) {
    destroy();
  }
  handle = other.handle;
  is_need_update = other.is_need_update;
}
Texture &Texture::operator=(const Texture &other) {
  if (is_need_update.use_count() == 1) {
    destroy();
  }
  handle = other.handle;
  is_need_update = other.is_need_update;
  return *this;
}
Texture::~Texture() {
  if (is_need_update.use_count() == 1) {
    destroy();
  }
}

bool Texture::load(std::string_view fileName) {
  *is_need_update = true;
  auto &surface = get(handle);
  auto *src_surface = ::IMG_Load_IO(
      (SDL_IOStream *)DataStream::open_as_rwops(AssetType::Texture, fileName),
      0);
  if (!src_surface) {
    Logger::error("Texture load failed.");
    return false;
  }
  memcpy(&surface, src_surface, sizeof(SDL_Surface));
  return true;
}
bool Texture::load_from_memory(std::vector<char> &buffer) {
  *is_need_update = true;
  auto &surface = get(handle);
  auto rw = std::unique_ptr<::SDL_IOStream, SDLObjectCloser>(
      ::SDL_IOFromMem(reinterpret_cast<void *>(buffer.data()), buffer.size()));
  if (!rw) {
    return false;
  }
  auto *src_surface = ::IMG_Load_IO(rw.get(), 0);
  if (!src_surface) {
    return false;
  }
  memcpy(&surface, src_surface, sizeof(SDL_Surface));

  return true;
}

void Texture::fill_color(const Color &color) {
  *is_need_update = true;
  auto &surface = get(handle);
  ::SDL_FillSurfaceRect(
      &surface, NULL,
      ::SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32), nullptr,
                    color.r * 255, color.g * 255, color.b * 255,
                    color.a * 255));
}
void Texture::blend_color(const Color &color) {
  *is_need_update = true;
  auto &surface = get(handle);
  SDL_SetSurfaceBlendMode(&surface, SDL_BLENDMODE_BLEND);
  SDL_SetSurfaceColorMod(&surface, color.r * 255, color.g * 255, color.b * 255);
  SDL_SetSurfaceAlphaMod(&surface, color.a * 255);
  auto *tmp = SDL_CreateSurface(surface.w, surface.h,
                                SDL_PixelFormat::SDL_PIXELFORMAT_RGBA8888);
  SDL_BlitSurface(&surface, NULL, (SDL_Surface *)handle, NULL);
}

Texture Texture::copy() {
  auto &src = get(handle);
  Texture dst_texture;
  auto &dst = get(dst_texture.handle);
  dst.w = src.w;
  dst.h = src.h;
  SDL_BlitSurface(&src, NULL, &dst, NULL);
  *dst_texture.is_need_update = true;
  return dst_texture;
}

Vector2 Texture::size() {
  auto &surface = get(handle);
  return Vector2(static_cast<float>(surface.w), static_cast<float>(surface.h));
}

HandleT Texture::create() {
  auto *surf = SDL_CreateSurface(1, 1, SDL_PIXELFORMAT_RGBA8888);
  HandleT handle = reinterpret_cast<HandleT>(surf);
  return handle;
}
void Texture::destroy() {
  if (!handle) {
    return;
  }
  SDL_Surface *surf = reinterpret_cast<SDL_Surface *>(handle);
  SDL_DestroySurface(surf);
  surf = nullptr;
  handle = NULL;
}

} // namespace sinen
