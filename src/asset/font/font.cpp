// std
#include <cassert>
#include <fstream>
#include <memory>
#include <optional>
#include <string_view>

// internal
#include "../texture/texture_data.hpp"
#include <asset/font/font.hpp>
#include <asset/model/mesh.hpp>
#include <asset/texture/texture.hpp>
#include <core/io/asset_io.hpp>
#include <core/logger/logger.hpp>
#include <graphics/rhi/rhi.hpp>
#include <math/color/color.hpp>

// external
#include <SDL3/SDL.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include "default/mplus-1p-medium.ttf.hpp"

namespace sinen {
struct CodepointRange {
  constexpr CodepointRange(uint32_t first, uint32_t last)
      : first(first), last(last), count(last - first + 1) {}
  uint32_t first;
  uint32_t last;
  uint32_t count;
};

constexpr CodepointRange HIRAGANA = {0x3042, 0x309E};
constexpr CodepointRange KATAKANA = {0x30A0, 0x30FF};
constexpr CodepointRange KANJI = {0x3000, 0x9FFF};
constexpr CodepointRange ASCII_FWIDTH = {0xFF01, 0xFF5E};
constexpr CodepointRange ASCII = {0x0020, 0x007F};

struct Font::Wrapper {
  std::vector<std::vector<stbtt_packedchar>> packedChar;
  Ptr<rhi::Texture> texture;
  uint32_t sheetSize = 0;
};
Font::Font() = default;
Font::Font(int32_t point, std::string_view file_name) {
  load(point, file_name);
}
Font::~Font() {}
static bool loadCore(const unsigned char *fontData,
                     std::vector<std::vector<stbtt_packedchar>> &pc,
                     Ptr<rhi::Texture> &texture, int pointSize, int sheetSize) {
  stbtt_pack_range ranges[5] = {};
  pc.resize(std::size(ranges));
  // Hiragana
  pc[0].resize(HIRAGANA.count);
  ranges[0].font_size = pointSize;
  ranges[0].first_unicode_codepoint_in_range = HIRAGANA.first;
  ranges[0].num_chars = HIRAGANA.count;
  ranges[0].chardata_for_range = pc[0].data();

  // Katakana
  pc[1].resize(KATAKANA.count);
  ranges[1].font_size = pointSize;
  ranges[1].first_unicode_codepoint_in_range = KATAKANA.first;
  ranges[1].num_chars = KATAKANA.count;
  ranges[1].chardata_for_range = pc[1].data();

  // ASCII WIDTH
  pc[2].resize(ASCII_FWIDTH.count);
  ranges[2].font_size = pointSize;
  ranges[2].first_unicode_codepoint_in_range = ASCII_FWIDTH.first;
  ranges[2].num_chars = ASCII_FWIDTH.count;
  ranges[2].chardata_for_range = pc[2].data();

  // Kanji
  pc[3].resize(KANJI.count);
  ranges[3].font_size = pointSize;
  ranges[3].first_unicode_codepoint_in_range = KANJI.first;
  ranges[3].num_chars = KANJI.count;
  ranges[3].chardata_for_range = pc[3].data();

  // ASCII
  pc[4].resize(ASCII.count);
  ranges[4].font_size = pointSize;
  ranges[4].first_unicode_codepoint_in_range = ASCII.first;
  ranges[4].num_chars = ASCII.count;
  ranges[4].chardata_for_range = pc[4].data();

  stbtt_pack_context spc;
  std::vector<unsigned char> atlasBitmap(sheetSize * sheetSize * 4);
  std::vector<unsigned char> temp(sheetSize * sheetSize);
  stbtt_PackBegin(&spc, temp.data(), sheetSize, sheetSize, 0, 1, NULL);
  stbtt_PackFontRanges(&spc, fontData, 0, ranges, std::size(ranges));
  stbtt_PackEnd(&spc);
  // 1ch -> 4ch (R8G8B8A8)
  for (int y = 0; y < sheetSize; ++y) {
    for (int x = 0; x < sheetSize; ++x) {
      int idx_gray = y * sheetSize + x;
      int idx_rgba = (y * sheetSize + x) * 4;

      unsigned char a = temp[idx_gray];

      atlasBitmap[idx_rgba + 0] = 255;
      atlasBitmap[idx_rgba + 1] = 255;
      atlasBitmap[idx_rgba + 2] = 255;
      atlasBitmap[idx_rgba + 3] = a;
    }
  }

  texture = createNativeTexture(atlasBitmap.data(),
                                rhi::TextureFormat::R8G8B8A8_UNORM, sheetSize,
                                sheetSize);

  return true;
}
bool Font::load(int pointSize) {
  this->font = std::make_unique<Wrapper>();
  this->m_size = pointSize;
  this->font->sheetSize = pointSize * 128;

  return loadCore(mplus1pMediumTtf, this->font->packedChar, this->font->texture,
                  pointSize, this->font->sheetSize);
}
bool Font::load(int pointSize, std::string_view fontName) {
  this->font = std::make_unique<Wrapper>();
  this->m_size = pointSize;
  this->font->sheetSize = pointSize * 128;
  return loadCore(reinterpret_cast<const unsigned char *>(
                      AssetIO::openAsString(fontName).data()),
                  this->font->packedChar, this->font->texture, pointSize,
                  this->font->sheetSize);
}
bool Font::loadFromPath(int pointSize, std::string_view path) { return true; }

void Font::unload() {}

void Font::resize(int point_size) {}
Texture Font::getAtlas() const {
  Texture texture;
  texture.texture = this->font->texture;
  return texture;
}

const char *utf8ToCodepoint(const char *p, uint32_t *out_cp) {
  unsigned char c = (unsigned char)*p;
  if (c < 0x80) {
    *out_cp = c;
    return p + 1;
  }
  if ((c & 0xE0) == 0xC0) {
    *out_cp = ((uint32_t)(c & 0x1F) << 6) | ((uint32_t)(p[1] & 0x3F));
    return p + 2;
  }
  if ((c & 0xF0) == 0xE0) {
    *out_cp = ((uint32_t)(c & 0x0F) << 12) | ((uint32_t)(p[1] & 0x3F) << 6) |
              ((uint32_t)(p[2] & 0x3F));
    return p + 3;
  }
  if ((c & 0xF8) == 0xF0) {
    *out_cp = ((uint32_t)(c & 0x07) << 18) | ((uint32_t)(p[1] & 0x3F) << 12) |
              ((uint32_t)(p[2] & 0x3F) << 6) | ((uint32_t)(p[3] & 0x3F));
    return p + 4;
  }
  *out_cp = 0xFFFD;
  return p + 1;
}

Mesh Font::getTextMesh(std::string_view text) const {

  assert(this->font);

  Mesh textMesh;
  float x = 0.f, y = 0.f;
  glm::vec2 yrange(99999, -99999);
  const char *p = text.data();
  while (*p) {
    uint32_t cp;
    const auto *next = utf8ToCodepoint(p, &cp);
    stbtt_aligned_quad q;
    uint32_t idx1 = 0, idx2 = 0;
    if (cp >= HIRAGANA.first && cp <= HIRAGANA.last) {
      // Hiragana
      idx1 = 0;
      idx2 = cp - HIRAGANA.first;
    } else if (cp >= KATAKANA.first && cp <= KATAKANA.last) {
      // Katakana
      idx1 = 1;
      idx2 = cp - KATAKANA.first;
    } else if (cp >= ASCII_FWIDTH.first && cp <= ASCII_FWIDTH.last) {
      // Zenkaku
      idx1 = 2;
      idx2 = cp - ASCII_FWIDTH.first;
    } else if (cp >= KANJI.first && cp <= KANJI.last) {
      // Kanji
      idx1 = 3;
      idx2 = cp - KANJI.first;
    } else if (cp >= ASCII.first && cp <= ASCII.last) {
      // ASCII
      idx1 = 4;
      idx2 = cp - ASCII.first;
    }

    uint32_t sheetSize = this->font->sheetSize;
    stbtt_GetPackedQuad(this->font->packedChar[idx1].data(), sheetSize,
                        sheetSize, idx2, &x, &y, &q, 1);
    uint32_t startIndex = textMesh.vertices.size();
    auto &vertices = textMesh.vertices;
    vertices.push_back(Vertex{
        {q.x0 * 4.f, -q.y0 * 4.f, 0}, {1, 1, 1}, {q.s0, q.t0}, {1, 1, 1, 1}});
    vertices.push_back(Vertex{
        {q.x0 * 4.f, -q.y1 * 4.f, 0}, {1, 1, 1}, {q.s0, q.t1}, {1, 1, 1, 1}});
    vertices.push_back(Vertex{
        {q.x1 * 4.f, -q.y0 * 4.f, 0}, {1, 1, 1}, {q.s1, q.t0}, {1, 1, 1, 1}});
    vertices.push_back(Vertex{
        {q.x1 * 4.f, -q.y1 * 4.f, 0}, {1, 1, 1}, {q.s1, q.t1}, {1, 1, 1, 1}});
    yrange.x = std::min(yrange.x, -q.y1 * 4.f);
    yrange.y = std::max(yrange.y, -q.y0 * 4.f);

    textMesh.indices.push_back(startIndex + 0);
    textMesh.indices.push_back(startIndex + 1);
    textMesh.indices.push_back(startIndex + 2);
    textMesh.indices.push_back(startIndex + 2);
    textMesh.indices.push_back(startIndex + 1);
    textMesh.indices.push_back(startIndex + 3);
    p = next;
  }
  for (auto &v : textMesh.vertices) {
    v.position.x -= x * 2.f;
    v.position.y -= (yrange.y - yrange.x) / 2.f;
  }
  textMesh.indexCount = textMesh.indices.size();
  return textMesh;
}
} // namespace sinen
