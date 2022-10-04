#include "../main/get_system.hpp"
#include "../texture/texture_system.hpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <cassert>
#include <color/color.hpp>
#include <font/font.hpp>
#include <io/dstream.hpp>
#include <logger/logger.hpp>
#include <texture/texture.hpp>

namespace sinen {
font::font(std::string_view file_name, int32_t point) {
  load(file_name, point);
}
font::~font() { unload(); }
bool font::load(std::string_view fontName, int pointSize) {
  this->font_name = fontName;
  this->point_size = pointSize;
  m_font = (void *)::TTF_OpenFontRW(
      (SDL_RWops *)dstream::open_as_rwops(asset_type::Font, this->font_name), 1,
      this->point_size);
  if (!m_font) {
    logger::error("%s", TTF_GetError());
    return false;
  }
  return (is_load = true);
}

void font::unload() {
  if (is_load) {
    ::TTF_CloseFont((TTF_Font *)m_font);
    is_load = false;
  }
}

void font::render_text(texture &tex, std::string_view text,
                       const color &_color) {
  *tex.is_need_update = true;
  // My Color to SDL_Color
  SDL_Color sdlColor;
  sdlColor.r = static_cast<Uint8>(_color.r * 255);
  sdlColor.g = static_cast<Uint8>(_color.g * 255);
  sdlColor.b = static_cast<Uint8>(_color.b * 255);
  sdlColor.a = static_cast<Uint8>(_color.a * 255);
  get_texture().remove(tex.handle);
  get_texture().move(
      tex.handle,
      std::unique_ptr<SDL_Surface, SDLObjectCloser>(
          ::TTF_RenderUTF8_Blended_Wrapped(
              (::TTF_Font *)m_font, std::string(text).c_str(), sdlColor, 0)));
}
} // namespace sinen
