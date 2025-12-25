// std
#include <cassert>
#include <cstddef>

// internal
#include "../texture/texture_data.hpp"
#include <asset/font/font.hpp>
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

constexpr CodepointRange asciiWidthRange = {0xFF01, 0xFF5E};
constexpr CodepointRange japaneseRange = {0x3000, 0x9FFF};
constexpr CodepointRange asciiRange = {0x0020, 0x007F};

struct Font::Wrapper {
  Array<Array<stbtt_packedchar>> packedChar;
  Ptr<rhi::Texture> texture;
  uint32_t sheetSize = 0;
};
Font::Font() = default;
Font::Font(int32_t point, StringView file_name) { load(point, file_name); }
Font::~Font() {}
static bool loadCore(const unsigned char *fontData,
                     Array<Array<stbtt_packedchar>> &pc,
                     Ptr<rhi::Texture> &texture, int pointSize, int sheetSize) {
  stbtt_pack_range ranges[3] = {};
  pc.resize(std::size(ranges));
  size_t index = 0;
  // ASCII WIDTH
  pc[index].resize(asciiWidthRange.count);
  ranges[index].font_size = pointSize;
  ranges[index].first_unicode_codepoint_in_range = asciiWidthRange.first;
  ranges[index].num_chars = asciiWidthRange.count;
  ranges[index].chardata_for_range = pc[index].data();
  index++;

  // Japanese
  pc[index].resize(japaneseRange.count);
  ranges[index].font_size = pointSize;
  ranges[index].first_unicode_codepoint_in_range = japaneseRange.first;
  ranges[index].num_chars = japaneseRange.count;
  ranges[index].chardata_for_range = pc[index].data();
  index++;

  // ASCII
  pc[index].resize(asciiRange.count);
  ranges[index].font_size = pointSize;
  ranges[index].first_unicode_codepoint_in_range = asciiRange.first;
  ranges[index].num_chars = asciiRange.count;
  ranges[index].chardata_for_range = pc[index].data();
  index++;

  stbtt_pack_context spc;
  Array<unsigned char> atlasBitmap(sheetSize * sheetSize * 4);
  Array<unsigned char> temp(sheetSize * sheetSize);
  stbtt_PackBegin(&spc, temp.data(), sheetSize, sheetSize, 0, 1, NULL);
  stbtt_PackFontRanges(&spc, fontData, 0, ranges, std::size(ranges));
  stbtt_PackEnd(&spc);
  // 1ch -> 4ch (R8G8B8A8)
  for (int y = 0; y < sheetSize; ++y) {
    for (int x = 0; x < sheetSize; ++x) {
      int idxGray = y * sheetSize + x;
      int idxRGBA = (y * sheetSize + x) * 4;

      unsigned char a = temp[idxGray];

      atlasBitmap[idxRGBA + 0] = 255;
      atlasBitmap[idxRGBA + 1] = 255;
      atlasBitmap[idxRGBA + 2] = 255;
      atlasBitmap[idxRGBA + 3] = a;
    }
  }

  texture = createNativeTexture(atlasBitmap.data(),
                                rhi::TextureFormat::R8G8B8A8_UNORM, sheetSize,
                                sheetSize);

  return true;
}
bool Font::load(int pointSize) {
  this->font = makeUnique<Wrapper>();
  this->m_size = pointSize;
  this->font->sheetSize = pointSize * 64;

  return loadCore(mplus1pMediumTtf, this->font->packedChar, this->font->texture,
                  pointSize, this->font->sheetSize);
}
bool Font::load(int pointSize, StringView fontName) {
  this->font = makeUnique<Wrapper>();
  this->m_size = pointSize;
  this->font->sheetSize = pointSize * 64;
  return loadCore(reinterpret_cast<const unsigned char *>(
                      AssetIO::openAsString(fontName).data()),
                  this->font->packedChar, this->font->texture, pointSize,
                  this->font->sheetSize);
}
bool Font::loadFromPath(int pointSize, StringView path) { return true; }

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

Mesh Font::getTextMesh(StringView text) const {

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
    if (cp >= asciiWidthRange.first && cp <= asciiWidthRange.last) {
      // Zenkaku
      idx1 = 0;
      idx2 = cp - asciiWidthRange.first;
    } else if (cp >= japaneseRange.first && cp <= japaneseRange.last) {
      // Japanese
      idx1 = 1;
      idx2 = cp - japaneseRange.first;
    } else if (cp >= asciiRange.first && cp <= asciiRange.last) {
      // ASCII
      idx1 = 2;
      idx2 = cp - asciiRange.first;
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
  return textMesh;
}
} // namespace sinen
