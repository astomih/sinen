#include "font_system.hpp"
#include <io/data_stream.hpp>
#include <logger/logger.hpp>

#include <SDL_ttf.h>
namespace sinen {
font_system::fonts_t font_system::m_fonts;
bool font_system::initialize() { return true; }
void font_system::shutdown() {
  for (auto &font : m_fonts) {
    for (auto &point : font.second) {
      ::TTF_CloseFont((::TTF_Font *)point.second);
    }
  }
  m_fonts.clear();
}
void *font_system::load(std::string_view file_name, int32_t point_size) {

  if (m_fonts.contains(std::string(file_name))) {
    if (m_fonts[std::string(file_name)].contains(point_size)) {
      return m_fonts[std::string(file_name)][point_size];
    }
  }

  m_fonts[std::string(file_name)][point_size] = (void *)::TTF_OpenFontRW(
      (SDL_RWops *)data_stream::open_as_rwops(asset_type::Font, file_name), 1,
      point_size);
  if (!m_fonts[std::string(file_name)][point_size]) {
    logger::error("Font error \"%s\"", TTF_GetError());
    return nullptr;
  }
  return m_fonts[std::string(file_name)][point_size];
}
void font_system::unload(std::string_view file_name, int32_t point_size) {
  if (m_fonts.contains(std::string(file_name))) {
    if (m_fonts[std::string(file_name)].contains(point_size)) {
      ::TTF_CloseFont(
          (::TTF_Font *)m_fonts[std::string(file_name)][point_size]);
      m_fonts[std::string(file_name)].erase(point_size);
    }
  }
}
} // namespace sinen
