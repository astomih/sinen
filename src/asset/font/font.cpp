// std
#include <cassert>
#include <fstream>
#include <memory>
#include <optional>
#include <string_view>

// internal
#include "../texture/texture_data.hpp"
#include "paranoixa/paranoixa.hpp"
#include <asset/font/font.hpp>
#include <asset/model/mesh.hpp>
#include <asset/texture/texture.hpp>
#include <core/io/asset_io.hpp>
#include <core/logger/logger.hpp>
#include <math/color/color.hpp>

// external
#include <SDL3/SDL.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include "default/mplus-1p-medium.ttf.hpp"

namespace sinen {
struct Font::Wrapper {
  stbtt_fontinfo fontInfo;
};
std::vector<glm::vec2>
getGlyphOutlineAsPolygon(stbtt_fontinfo &font, int glyph_index, float scale,
                         float bezierTessellationStep = 0.05f) {
  std::vector<glm::vec2> polygon;

  stbtt_vertex *vertices = nullptr;
  int num_verts = stbtt_GetGlyphShape(&font, glyph_index, &vertices);
  if (num_verts <= 0 || !vertices) {
    return polygon;
  }

  glm::vec2 current = {0, 0};
  for (int i = 0; i < num_verts; ++i) {
    stbtt_vertex &v = vertices[i];

    switch (v.type) {
    case STBTT_vmove:
      polygon.clear();
      current.x = v.x * scale;
      current.y = v.y * scale;
      polygon.push_back(current);
      break;

    case STBTT_vline: {
      glm::vec2 p = {v.x * scale, v.y * scale};
      polygon.push_back(p);
      current = p;
      break;
    }

    case STBTT_vcurve: {
      glm::vec2 control = {v.cx * scale, v.cy * scale};
      glm::vec2 end = {v.x * scale, v.y * scale};

      for (float t = bezierTessellationStep; t <= 1.0f;
           t += bezierTessellationStep) {
        float u = 1.0f - t;
        glm::vec2 p;
        p.x = u * u * current.x + 2 * u * t * control.x + t * t * end.x;
        p.y = u * u * current.y + 2 * u * t * control.y + t * t * end.y;
        polygon.push_back(p);
      }
      current = end;
      break;
    }

    case STBTT_vcubic:
      break;

    default:
      break;
    }
  }

  stbtt_FreeShape(&font, vertices);

  for (auto &p : polygon) {
    p.y = -p.y;
  }

  return polygon;
}

static float cross(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c) {
  float abx = b.x - a.x;
  float aby = b.y - a.y;
  float acx = c.x - a.x;
  float acy = c.y - a.y;
  return abx * acy - aby * acx;
}

static bool isPointInTriangle(const glm::vec2 &p, const glm::vec2 &a,
                              const glm::vec2 &b, const glm::vec2 &c) {
  float c1 = cross(a, b, p);
  float c2 = cross(b, c, p);
  float c3 = cross(c, a, p);

  bool hasNeg = (c1 < 0) || (c2 < 0) || (c3 < 0);
  bool hasPos = (c1 > 0) || (c2 > 0) || (c3 > 0);

  return !(hasNeg && hasPos);
}

static bool isConvex(const glm::vec2 &prev, const glm::vec2 &curr,
                     const glm::vec2 &next) {
  return cross(prev, curr, next) > 0.0f;
}

bool triangulateEarClipping(const std::vector<glm::vec2> &polygon, Mesh &mesh) {
  const size_t n = polygon.size();
  if (n < 3)
    return false;

  mesh.vertices.resize(n);
  for (int i = 0; i < n; i++) {
    mesh.vertices[i].position = glm::vec3(polygon[i].x, polygon[i].y, 0);
  }
  mesh.indices.clear();

  std::vector<int> indices(n);
  for (size_t i = 0; i < n; ++i)
    indices[i] = (int)i;

  int guard = 0;
  while (indices.size() > 3 && guard < 10000) {
    bool earFound = false;

    for (size_t i = 0; i < indices.size(); ++i) {
      int i0 = indices[(i + indices.size() - 1) % indices.size()];
      int i1 = indices[i];
      int i2 = indices[(i + 1) % indices.size()];

      const glm::vec3 &a = mesh.vertices[i0].position;
      const glm::vec3 &b = mesh.vertices[i1].position;
      const glm::vec3 &c = mesh.vertices[i2].position;

      if (!isConvex(a, b, c)) {
        continue;
      }

      bool containPoint = false;
      for (size_t j = 0; j < indices.size(); ++j) {
        int idx = indices[j];
        if (idx == i0 || idx == i1 || idx == i2)
          continue;

        const glm::vec3 &p = mesh.vertices[idx].position;
        if (isPointInTriangle(p, a, b, c)) {
          containPoint = true;
          break;
        }
      }

      if (containPoint) {
        continue;
      }

      mesh.indices.push_back((uint32_t)i0);
      mesh.indices.push_back((uint32_t)i1);
      mesh.indices.push_back((uint32_t)i2);

      indices.erase(indices.begin() + i);
      earFound = true;
      break;
    }

    if (!earFound) {
      std::cerr << "Ear clipping failed: no ear found.\n";
      return false;
    }

    ++guard;
  }

  if (indices.size() == 3) {
    mesh.indices.push_back((uint32_t)indices[0]);
    mesh.indices.push_back((uint32_t)indices[1]);
    mesh.indices.push_back((uint32_t)indices[2]);
  }

  return true;
}

std::optional<Mesh> buildGlyphMesh(stbtt_fontinfo &font, uint32_t codepoint,
                                   float pixelHeight) {

  float scale = stbtt_ScaleForPixelHeight(&font, pixelHeight);

  int glyph_index = stbtt_FindGlyphIndex(&font, codepoint);
  if (glyph_index == 0) {
    std::cerr << "Glyph not found for codepoint: " << codepoint << "\n";
    return std::nullopt;
  }

  std::vector<glm::vec2> polygon =
      getGlyphOutlineAsPolygon(font, glyph_index, scale);
  if (polygon.size() < 3) {
    std::cerr << "Glyph outline polygon too small.\n";
    return std::nullopt;
  }

  Mesh mesh;
  if (!triangulateEarClipping(polygon, mesh)) {
    std::cerr << "Triangulation failed.\n";
    return std::nullopt;
  }

  return mesh;
}
Font::Font() = default;
Font::Font(int32_t point, std::string_view file_name) {
  load(point, file_name);
}
Font::~Font() {}
bool Font::load(int pointSize) {
  stbtt_fontinfo fontInfo;
  if (!stbtt_InitFont(&fontInfo, mplus_1p_medium_ttf,
                      stbtt_GetFontOffsetForIndex(mplus_1p_medium_ttf, 0))) {
    return false;
  }
  this->font->fontInfo = fontInfo;
  this->m_size = pointSize;
  return true;
}
bool Font::load(int pointSize, std::string_view fontName) { return true; }
bool Font::loadFromPath(int pointSize, std::string_view path) { return true; }

void Font::unload() {}

void Font::resize(int point_size) {}

Mesh Font::getTextMesh(std::string_view text) {
  Mesh textMesh;
  float offset = 0.f;
  for (int i = 0; i < text.length(); ++i) {
    auto result = buildGlyphMesh(this->font->fontInfo, text.at(i), m_size);
    if (result.has_value()) {
      Mesh mesh = result.value();
      for (auto &v : mesh.vertices) {
        v.position.x += offset;
      }
      uint32_t startIndex = textMesh.vertices.size();
      textMesh.vertices.insert(textMesh.vertices.end(), mesh.vertices.begin(),
                               mesh.vertices.end());
      for (auto &ind : mesh.indices) {
        ind += startIndex;
      }
      textMesh.indices.insert(textMesh.indices.end(), mesh.indices.begin(),
                              mesh.indices.end());
      offset += 1.f; //?
    }
  }
  return textMesh;
}
} // namespace sinen
