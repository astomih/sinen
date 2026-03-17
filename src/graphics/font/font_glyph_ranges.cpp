#include "font_glyph_ranges.hpp"

namespace sinen::font {
namespace {

Array<UInt32> buildGlyphCodepoints(const ImWchar *ranges) {
  Array<UInt32> codepoints;
  for (int i = 0; ranges[i] != 0; i += 2) {
    for (UInt32 cp = ranges[i]; cp <= ranges[i + 1]; ++cp) {
      codepoints.push_back(cp);
    }
  }
  return codepoints;
}

} // namespace

const ImWchar *defaultJapaneseGlyphRangesForImGui() {
  static ImFontAtlas atlas;
  return atlas.GetGlyphRangesJapanese();
}

const Array<UInt32> &defaultGlyphCodepoints() {
  static const Array<UInt32> codepoints =
      buildGlyphCodepoints(defaultJapaneseGlyphRangesForImGui());
  return codepoints;
}

} // namespace sinen::font
