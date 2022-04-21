#include "../texture/texture_system.hpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <cassert>
#include <color/color.hpp>
#include <font/font.hpp>
#include <io/dstream.hpp>
#include <logger/logger.hpp>
#include <manager/manager.hpp>
#include <texture/texture.hpp>

namespace nen {
font::font(std::string_view file_name, int32_t point) {
  LoadFromFile(file_name, point);
}
font::~font() { Unload(); }
bool font::LoadFromFile(std::string_view fontName, int pointSize) {
  this->fontName = fontName;
  this->pointSize = pointSize;
  m_font = (void *)::TTF_OpenFontRW(
      (SDL_RWops *)dstream::open_as_rwops(asset_type::Font, this->fontName), 1,
      this->pointSize);
  if (!m_font) {
    logger::error("%s", TTF_GetError());
    return false;
  }
  return (isLoad = true);
}

void font::Unload() {
  if (isLoad) {
    ::TTF_CloseFont((TTF_Font *)m_font);
    isLoad = false;
  }
}

void font::RenderText(texture &tex, std::string_view text,
                      const color &_color) {
  // My Color to SDL_Color
  SDL_Color sdlColor;
  sdlColor.r = static_cast<Uint8>(_color.r * 255);
  sdlColor.g = static_cast<Uint8>(_color.g * 255);
  sdlColor.b = static_cast<Uint8>(_color.b * 255);
  sdlColor.a = static_cast<Uint8>(_color.a * 255);
  get_texture_system().remove(tex.handle);
  get_texture_system().move(
      tex.handle,
      std::unique_ptr<SDL_Surface, SDLObjectCloser>(::TTF_RenderUTF8_Blended(
          (::TTF_Font *)m_font, std::string(text).c_str(), sdlColor)));
}
} // namespace nen
