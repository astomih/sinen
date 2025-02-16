// std
#include <cassert>
#include <string_view>

// internal
#include <color/color.hpp>
#include <font/font.hpp>
#include <io/data_stream.hpp>
#include <logger/logger.hpp>

// external
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <texture/texture.hpp>

namespace sinen {
Font::Font(std::string_view file_name, int32_t point) {
  load(file_name, point);
}
Font::~Font() {}
bool Font::load(std::string_view fontName, int pointSize) {
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
  auto *surface =
      (::TTF_RenderText_Blended_Wrapped(ttf_font, std::string(text).c_str(),
                                        std::string(text).size(), sdlColor, 0));
  assert(surface != nullptr && "Failed to render text");
  SDL_Surface *handle = reinterpret_cast<SDL_Surface *>(tex.handle);
  // swap
  {
    SDL_Surface tmp = *handle;
    *handle = *surface;
    *surface = tmp;
  }
  SDL_DestroySurface(surface);
}
} // namespace sinen
