#include "../Texture/texture_system.hpp"
#include <Color/Color.hpp>
#include <Font/Font.hpp>
#include <IO/AssetReader.hpp>
#include <Logger/Logger.hpp>
#include <SDL.h>
#include <SDL_ttf.h>
#include <Texture/Texture.hpp>
#include <cassert>
#include <manager/manager.hpp>

namespace nen {
font::font(std::string_view file_name, int32_t point) {
  LoadFromFile(file_name, point);
}
font::~font() { Unload(); }
bool font::LoadFromFile(std::string_view fontName, int pointSize) {
  this->fontName = fontName;
  this->pointSize = pointSize;
  m_font = (void *)::TTF_OpenFontRW(
      (SDL_RWops *)data_io::LoadAsRWops(asset_type::Font, this->fontName), 1,
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
