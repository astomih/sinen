#ifndef SINEN_FONT_GLYPH_RANGES_HPP
#define SINEN_FONT_GLYPH_RANGES_HPP

#include <core/data/array.hpp>
#include <core/def/types.hpp>

#include <imgui.h>

namespace sinen::font {

const ImWchar *defaultJapaneseGlyphRangesForImGui();
const Array<UInt32> &defaultGlyphCodepoints();

} // namespace sinen::font

#endif // SINEN_FONT_GLYPH_RANGES_HPP
