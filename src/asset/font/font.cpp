// std
#include <cassert>
#include <memory>
#include <string_view>

// internal
#include "../texture/texture_data.hpp"
#include "paranoixa/paranoixa.hpp"
#include <asset/font/font.hpp>
#include <asset/texture/texture.hpp>
#include <core/io/asset_io.hpp>
#include <core/logger/logger.hpp>
#include <math/color/color.hpp>

// external
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "default/mplus-1p-medium.ttf.hpp"

namespace sinen {
Font::Font(int32_t point, std::string_view file_name) {
  load(point, file_name);
}
Font::~Font() {}
bool Font::load(int pointSize) {
  return true;
}
bool Font::load(int pointSize, std::string_view fontName) {
  return true;
}
bool Font::loadFromPath(int pointSize, std::string_view path) {
  return true;
}

void Font::unload() {
}

void Font::resize(int point_size) {
}

void Font::renderText(Texture &tex, const std::string &text,
                      const Color &_color) {
}
} // namespace sinen
