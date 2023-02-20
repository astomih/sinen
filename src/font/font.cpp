#include <SDL.h>
#include <SDL_ttf.h>
#include <cassert>
#include <color/color.hpp>
#include <font/font.hpp>
#include <io/data_stream.hpp>
#include <logger/logger.hpp>
#include <string_view>
#include <texture/texture.hpp>

namespace sinen {
font::font(std::string_view file_name, int32_t point) {
  load(file_name, point);
}
font::~font() {}
bool font::load(std::string_view fontName, int pointSize) {
  this->font_name = fontName;
  this->point_size = pointSize;
  m_font = (void *)::TTF_OpenFontRW(
      (SDL_RWops *)data_stream::open_as_rwops(asset_type::Font, fontName), 1,
      pointSize);

  if (!m_font) {
    return false;
  }
  is_load = true;
  return true;
}

void font::unload() {
  if (is_load) {
    ::TTF_CloseFont((::TTF_Font *)m_font);
  }
}

void font::resize(int point_size) {
  if (!is_load) {
    logger::error("Font is not loaded");
    return;
  }
  TTF_SetFontSize(reinterpret_cast<TTF_Font *>(this->m_font), point_size);
}

void font::render_text(texture &tex, std::string_view text,
                       const color &_color) {
  if (!is_load || !m_font) {
    logger::error("Font is not loaded");
    return;
  }
  *tex.is_need_update = true;
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
  auto *surface = (::TTF_RenderUTF8_Blended_Wrapped(
      ttf_font, std::string(text).c_str(), sdlColor, 0));
  if (!surface) {
    logger::error("Failed to render text: %s", ::TTF_GetError());
  }
  SDL_Surface *handle = reinterpret_cast<SDL_Surface *>(tex.handle);
  // swap
  {
    SDL_Surface tmp = *handle;
    *handle = *surface;
    *surface = tmp;
  }
  SDL_FreeSurface(surface);
}
} // namespace sinen
