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

#include "default/mplus-1p-medium.ttf.h"

namespace sinen {
Font::Font(int32_t point, std::string_view file_name) {
  load_from_file(point, file_name);
}
Font::~Font() {}
bool Font::load(int pointSize) {
  this->m_size = pointSize;
  // Load the default font from the embedded resource
  auto *rw = SDL_IOFromConstMem(mplus_1p_medium_ttf, mplus_1p_medium_ttf_len);
  m_font = (void *)::TTF_OpenFontIO(rw, 1, pointSize);
  return is_loaded();
}
bool Font::load_from_file(int pointSize, std::string_view fontName) {
  this->m_size = pointSize;
  m_font = (void *)::TTF_OpenFontIO(
      (SDL_IOStream *)DataStream::open_as_rwops(AssetType::Font, fontName), 1,
      pointSize);
  return is_loaded();
}

void Font::unload() {
  if (is_loaded()) {
    ::TTF_CloseFont((::TTF_Font *)m_font);
  }
}

void Font::resize(int point_size) {
  if (!is_loaded()) {
    Logger::error("Font is not loaded");
    return;
  }
  TTF_SetFontSize(reinterpret_cast<TTF_Font *>(this->m_font), point_size);
}

void Font::render_text(Texture &tex, std::string_view text,
                       const Color &_color) {
  if (!is_loaded()) {
    Logger::error("Font is not loaded");
    return;
  }
  // SinenEngine Color to SDL_Color
  SDL_Color sdlColor;
  sdlColor.r = static_cast<Uint8>(_color.r * 255);
  sdlColor.g = static_cast<Uint8>(_color.g * 255);
  sdlColor.b = static_cast<Uint8>(_color.b * 255);
  sdlColor.a = static_cast<Uint8>(_color.a * 255);
  if (sdlColor.a == 0) {
    tex.fill_color(_color);
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
