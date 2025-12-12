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
#define NUM_HIRA (0x309F - 0x3040 + 1)
#define NUM_KATA (0x30FF - 0x30A0 + 1)
#define NUM_ASCII_FWIDTH (0xFF5E - 0xFF01 + 1)
static constexpr int NUM_KANJI = 0x9FFF - 0x3000 + 1;
#define NUM_ASCII (0x007F - 0x0020 + 1)

struct Font::Wrapper {
  std::vector<std::vector<stbtt_packedchar>> packedChar;
  rhi::Ptr<rhi::Texture> texture;
  uint32_t sheetSize = 0;
};
Font::Font() = default;
Font::Font(int32_t point, std::string_view file_name) {
  load(point, file_name);
}
Font::~Font() {}
bool Font::load(int pointSize) {
  this->font = std::make_unique<Wrapper>();
  this->font->packedChar.resize(5);
  auto &pc = this->font->packedChar;
  stbtt_pack_range ranges[5] = {};
  // Hiragana
  pc[0].resize(NUM_HIRA);
  ranges[0].font_size = pointSize;
  ranges[0].first_unicode_codepoint_in_range = 0x3040;
  ranges[0].num_chars = NUM_HIRA;
  ranges[0].chardata_for_range = pc[0].data();

  // Katakana
  pc[1].resize(NUM_KATA);
  ranges[1].font_size = pointSize;
  ranges[1].first_unicode_codepoint_in_range = 0x30A0;
  ranges[1].num_chars = NUM_KATA;
  ranges[1].chardata_for_range = pc[1].data();

  pc[2].resize(NUM_ASCII_FWIDTH);
  ranges[2].font_size = pointSize;
  ranges[2].first_unicode_codepoint_in_range = 0xFF01;
  ranges[2].num_chars = NUM_ASCII_FWIDTH;
  ranges[2].chardata_for_range = pc[2].data();

  // Kanji
  pc[3].resize(NUM_KANJI);
  ranges[3].font_size = pointSize;
  ranges[3].first_unicode_codepoint_in_range = 0x3000;
  ranges[3].num_chars = NUM_KANJI;
  ranges[3].chardata_for_range = pc[3].data();

  // ASCII
  pc[4].resize(NUM_ASCII);
  ranges[4].font_size = pointSize;
  ranges[4].first_unicode_codepoint_in_range = 0x0020;
  ranges[4].num_chars = NUM_ASCII;
  ranges[4].chardata_for_range = pc[4].data();

  this->m_size = pointSize;
  stbtt_pack_context spc;
  this->font->sheetSize = pointSize * 128;
  auto &sheetSize = this->font->sheetSize;
  std::vector<unsigned char> atlasBitmap(sheetSize * sheetSize * 4);
  std::vector<unsigned char> temp(sheetSize * sheetSize);
  stbtt_PackBegin(&spc, temp.data(), sheetSize, sheetSize, 0, 1, NULL);
  stbtt_PackSetOversampling(&spc, 2, 2);
  stbtt_PackFontRanges(&spc, mplus_1p_medium_ttf, 0, ranges, 5);
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

  this->font->texture = createNativeTexture(atlasBitmap.data(),
                                            rhi::TextureFormat::R8G8B8A8_UNORM,
                                            sheetSize, sheetSize);

  return true;
}
bool Font::load(int pointSize, std::string_view fontName) { return true; }
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
  } else if ((c & 0xE0) == 0xC0) {
    *out_cp = ((uint32_t)(c & 0x1F) << 6) | ((uint32_t)(p[1] & 0x3F));
    return p + 2;
  } else if ((c & 0xF0) == 0xE0) {
    *out_cp = ((uint32_t)(c & 0x0F) << 12) | ((uint32_t)(p[1] & 0x3F) << 6) |
              ((uint32_t)(p[2] & 0x3F));
    return p + 3;
  } else if ((c & 0xF8) == 0xF0) {
    *out_cp = ((uint32_t)(c & 0x07) << 18) | ((uint32_t)(p[1] & 0x3F) << 12) |
              ((uint32_t)(p[2] & 0x3F) << 6) | ((uint32_t)(p[3] & 0x3F));
    return p + 4;
  } else {
    *out_cp = 0xFFFD;
    return p + 1;
  }
}

Mesh Font::getTextMesh(std::string_view text) const {

  assert(this->font);

  Mesh textMesh;
  float x = 0.f, y = 0.f;
  const char *p = text.data();
  while (*p) {
    uint32_t cp;
    const auto *next = utf8ToCodepoint(p, &cp);
    stbtt_aligned_quad q;
    uint32_t idx1 = 0, idx2 = 0;
    if (cp >= 0x3040 && cp <= 0x309F) {
      // Hiragana
      idx1 = 0;
      idx2 = cp - 0x3040;
    } else if (cp >= 0x30A0 && cp <= 0x30FF) {
      // Katakana
      idx1 = 1;
      idx2 = cp - 0x30A0;
    } else if (cp >= 0xFF01 && cp <= 0xFF5E) {
      // Zenkaku
      idx1 = 2;
      idx2 = cp - 0xFF01;
    } else if (cp >= 0x3000 && cp <= 0x9FFF) {
      // Kanji
      idx1 = 3;
      idx2 = cp - 0x3000;
    } else if (cp >= 0x0020 && cp <= 0x007F) {
      // ASCII
      idx1 = 4;
      idx2 = cp - 0x0020;
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
  }
  textMesh.indexCount = textMesh.indices.size();
  return textMesh;
}
} // namespace sinen
