// std
#include <cassert>
#include <memory>
#include <string_view>

// internal
#include "../texture/texture_data.hpp"
#include "paranoixa/paranoixa.hpp"
#include <asset/font/font.hpp>
#include <asset/texture/texture.hpp>
#include <core/io/data_stream.hpp>
#include <core/logger/logger.hpp>
#include <math/color/color.hpp>

// external
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "default/mplus-1p-medium.ttf.hpp"

namespace sinen {
Font::Font(int32_t point, std::string_view file_name) {
  Load(point, file_name);
}
Font::~Font() {}
bool Font::Load(int pointSize) {
  this->m_size = pointSize;
  // Load the default font from the embedded resource
  auto *rw = SDL_IOFromConstMem(mplus_1p_medium_ttf, mplus_1p_medium_ttf_len);
  m_font = (void *)::TTF_OpenFontIO(rw, 1, pointSize);
  return IsLoaded();
}
bool Font::Load(int pointSize, std::string_view fontName) {
  this->m_size = pointSize;
  m_font = (void *)::TTF_OpenFontIO(
      (SDL_IOStream *)DataStream::OpenAsRWOps(AssetType::Font, fontName), 1,
      pointSize);
  return IsLoaded();
}

void Font::Unload() {
  if (IsLoaded()) {
    ::TTF_CloseFont((::TTF_Font *)m_font);
  }
}

void Font::Resize(int point_size) {
  if (!IsLoaded()) {
    Logger::Error("Font is not loaded");
    return;
  }
  TTF_SetFontSize(reinterpret_cast<TTF_Font *>(this->m_font), point_size);
}

void Font::RenderText(Texture &tex, std::string_view text,
                      const Color &_color) {
  if (!IsLoaded()) {
    Logger::Error("Font is not loaded");
    return;
  }
  // SinenEngine Color to SDL_Color
  SDL_Color sdlColor;
  sdlColor.r = static_cast<Uint8>(_color.r * 255);
  sdlColor.g = static_cast<Uint8>(_color.g * 255);
  sdlColor.b = static_cast<Uint8>(_color.b * 255);
  sdlColor.a = static_cast<Uint8>(_color.a * 255);
  if (sdlColor.a == 0) {
    tex.FillColor(_color);
    return;
  }
  auto *ttf_font = reinterpret_cast<::TTF_Font *>(m_font);
  auto *pSurface =
      (::TTF_RenderText_Blended_Wrapped(ttf_font, std::string(text).c_str(),
                                        std::string(text).size(), sdlColor, 0));
  assert(pSurface != nullptr && "Failed to render text");
  auto texdata = GetTexData(tex.textureData);
  SDL_Surface *temp = (texdata->pSurface);
  bool isUpdate = temp->w == pSurface->w && temp->h == pSurface->h;
  texdata->pSurface = pSurface;
  if (texdata->texture && isUpdate) {
    UpdateNativeTexture(texdata->texture, pSurface);
  } else {
    texdata->texture = CreateNativeTexture(pSurface);
  }
  SDL_DestroySurface(temp);
}
} // namespace sinen
