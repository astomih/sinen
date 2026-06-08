// std
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <exception>
#include <memory>
#include <mutex>
#include <thread>

// internal
#include <core/data/array.hpp>
#include <core/data/hashmap.hpp>
#include <gpu/gpu.hpp>
#include <graphics/font/font.hpp>
#include <graphics/texture/texture.hpp>
#include <math/color/color.hpp>
#include <math/geometry/mesh.hpp>
#include <math/math.hpp>
#include <platform/io/asset_reader.hpp>

// external
#include <msdfgen.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_RASTERIZER_VERSION 2
#include <stb_truetype.h>

#include "default/mplus-1p-medium.ttf.hpp"

namespace sinen {
namespace {
constexpr UInt32 kFallbackAsciiCodepoint = '?';
constexpr UInt32 kReplacementCodepoint = 0xFFFD;
constexpr int kAtlasSizes[] = {64, 128, 256, 512, 1024, 2048, 4096, 8192};
constexpr int kGlyphPadding = 1;
constexpr int kDefaultMsdfGlyphPixelHeight = 32;
constexpr int kDefaultMsdfPixelRange = 6;
constexpr int kMinMsdfPixelRange = 2;
constexpr int kMsdfAtlasGutter = 1;
constexpr float kAtlasEstimateSlack = 1.6f;

struct PackedAtlasData {
  Array<int> codepoints;
  Array<stbtt_packedchar> packedChars;
  Hashmap<UInt32, UInt32> glyphLookup;
  Array<unsigned char> atlasBitmap;
  gpu::TextureFormat textureFormat = gpu::TextureFormat::R8_UNORM;
  int channels = 1;
  int distanceFieldRange = 1;
  UInt32 sheetSize = 0;
  bool success = false;
};

int msdfPixelRangeForSize(int pointSize) {
  const float range = static_cast<float>(pointSize) *
                      static_cast<float>(kDefaultMsdfPixelRange) /
                      static_cast<float>(kDefaultMsdfGlyphPixelHeight);
  return std::max(kMinMsdfPixelRange, static_cast<int>(std::ceil(range)));
}

int estimateInitialAtlasSize(const stbtt_fontinfo &fontInfo, int pointSize,
                             const Array<int> &codepoints, FontMethod method,
                             int msdfPixelRange) {
  const float scale =
      stbtt_ScaleForPixelHeight(&fontInfo, static_cast<float>(pointSize));
  uint64_t estimatedArea = 0;
  for (int cp : codepoints) {
    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBoxSubpixel(&fontInfo, cp, scale, scale, 0.0f, 0.0f,
                                        &x0, &y0, &x1, &y1);
    int width = std::max(0, x1 - x0) + kGlyphPadding;
    int height = std::max(0, y1 - y0) + kGlyphPadding;
    if (method == FontMethod::MSDF && width > 0 && height > 0) {
      width += msdfPixelRange * 2;
      height += msdfPixelRange * 2;
      width += kMsdfAtlasGutter * 2;
      height += kMsdfAtlasGutter * 2;
    }
    estimatedArea +=
        static_cast<uint64_t>(width) * static_cast<uint64_t>(height);
  }

  const double targetArea =
      static_cast<double>(estimatedArea) * kAtlasEstimateSlack;
  for (int sheetSize : kAtlasSizes) {
    const double sheetArea = static_cast<double>(sheetSize) * sheetSize;
    if (targetArea <= sheetArea) {
      return sheetSize;
    }
  }
  return kAtlasSizes[sizeof(kAtlasSizes) / sizeof(kAtlasSizes[0]) - 1];
}

msdfgen::Point2 toMsdfPoint(const stbtt_vertex &v, float scale) {
  return msdfgen::Point2(static_cast<double>(v.x) * scale,
                         -static_cast<double>(v.y) * scale);
}

bool almostEqual(const msdfgen::Point2 &a, const msdfgen::Point2 &b) {
  constexpr double epsilon = 0.0001;
  return std::abs(a.x - b.x) < epsilon && std::abs(a.y - b.y) < epsilon;
}

msdfgen::Shape makeGlyphShape(const stbtt_fontinfo &fontInfo, int glyphIndex,
                              float scale) {
  msdfgen::Shape shape;
  stbtt_vertex *vertices = nullptr;
  const int vertexCount = stbtt_GetGlyphShape(&fontInfo, glyphIndex, &vertices);
  if (vertexCount <= 0 || vertices == nullptr) {
    if (vertices != nullptr) {
      stbtt_FreeShape(&fontInfo, vertices);
    }
    return shape;
  }

  msdfgen::Contour *contour = nullptr;
  msdfgen::Point2 contourStart;
  msdfgen::Point2 current;
  bool hasCurrent = false;

  auto closeContour = [&]() {
    if (contour == nullptr) {
      return;
    }
    if (contour->edges.empty()) {
      shape.contours.pop_back();
    } else if (!almostEqual(current, contourStart)) {
      contour->addEdge(msdfgen::EdgeHolder(current, contourStart));
    }
    contour = nullptr;
    hasCurrent = false;
  };

  for (int i = 0; i < vertexCount; ++i) {
    const stbtt_vertex &v = vertices[i];
    switch (v.type) {
    case STBTT_vmove:
      closeContour();
      contour = &shape.addContour();
      current = toMsdfPoint(v, scale);
      contourStart = current;
      hasCurrent = true;
      break;
    case STBTT_vline: {
      if (!hasCurrent || contour == nullptr) {
        break;
      }
      const msdfgen::Point2 p = toMsdfPoint(v, scale);
      if (!almostEqual(current, p)) {
        contour->addEdge(msdfgen::EdgeHolder(current, p));
      }
      current = p;
      break;
    }
    case STBTT_vcurve: {
      if (!hasCurrent || contour == nullptr) {
        break;
      }
      const msdfgen::Point2 control(static_cast<double>(v.cx) * scale,
                                    -static_cast<double>(v.cy) * scale);
      const msdfgen::Point2 p = toMsdfPoint(v, scale);
      if (!almostEqual(current, p)) {
        contour->addEdge(msdfgen::EdgeHolder(current, control, p));
      }
      current = p;
      break;
    }
    case STBTT_vcubic: {
      if (!hasCurrent || contour == nullptr) {
        break;
      }
      const msdfgen::Point2 control0(static_cast<double>(v.cx) * scale,
                                     -static_cast<double>(v.cy) * scale);
      const msdfgen::Point2 control1(static_cast<double>(v.cx1) * scale,
                                     -static_cast<double>(v.cy1) * scale);
      const msdfgen::Point2 p = toMsdfPoint(v, scale);
      if (!almostEqual(current, p)) {
        contour->addEdge(msdfgen::EdgeHolder(current, control0, control1, p));
      }
      current = p;
      break;
    }
    default:
      break;
    }
  }
  closeContour();
  stbtt_FreeShape(&fontInfo, vertices);

  if (!shape.contours.empty()) {
    shape.orientContours();
    shape.normalize();
    msdfgen::edgeColoringSimple(shape, 3.0);
  }
  return shape;
}

unsigned char msdfFloatToByte(float value) {
  const float clamped = std::clamp(value, 0.0f, 1.0f);
  return static_cast<unsigned char>(clamped * 255.0f + 0.5f);
}

UInt32 selectFallbackGlyphIndex(const Hashmap<UInt32, UInt32> &glyphLookup) {
  if (auto it = glyphLookup.find(kFallbackAsciiCodepoint);
      it != glyphLookup.end()) {
    return it->second;
  }
  if (auto it = glyphLookup.find(kReplacementCodepoint);
      it != glyphLookup.end()) {
    return it->second;
  }
  return 0;
}

std::size_t msdfWorkerCount(std::size_t glyphCount) {
  if (glyphCount < 8) {
    return 1;
  }
  const unsigned int hardwareThreads = std::thread::hardware_concurrency();
  const std::size_t workerThreads =
      hardwareThreads > 1 ? static_cast<std::size_t>(hardwareThreads - 1) : 1;
  return std::clamp<std::size_t>(workerThreads, 1, glyphCount);
}

bool tryPackAtlas(const unsigned char *fontData, int pointSize, int sheetSize,
                  const Array<int> &codepoints, PackedAtlasData &result) {
  result.codepoints.clear();
  result.packedChars.clear();
  result.glyphLookup.clear();
  result.atlasBitmap.clear();

  result.codepoints = codepoints;

  result.packedChars.resize(result.codepoints.size());

  stbtt_pack_range range = {};
  range.font_size = static_cast<float>(pointSize);
  range.first_unicode_codepoint_in_range = 0;
  range.array_of_unicode_codepoints = result.codepoints.data();
  range.num_chars = static_cast<int>(result.codepoints.size());
  range.chardata_for_range = result.packedChars.data();

  Array<unsigned char> monoAtlas(sheetSize * sheetSize);
  stbtt_pack_context spc = {};
  if (!stbtt_PackBegin(&spc, monoAtlas.data(), sheetSize, sheetSize, 0, 1,
                       nullptr)) {
    return false;
  }

  stbtt_PackSetOversampling(&spc, 1, 1);
  stbtt_PackSetSkipMissingCodepoints(&spc, 1);
  const int packed = stbtt_PackFontRanges(&spc, fontData, 0, &range, 1);
  stbtt_PackEnd(&spc);
  if (!packed) {
    return false;
  }

  result.atlasBitmap = std::move(monoAtlas);
  result.textureFormat = gpu::TextureFormat::R8_UNORM;
  result.channels = 1;

  result.glyphLookup.reserve(result.codepoints.size());
  for (UInt32 i = 0; i < result.codepoints.size(); ++i) {
    result.glyphLookup.emplace(static_cast<UInt32>(result.codepoints[i]), i);
  }

  result.sheetSize = static_cast<UInt32>(sheetSize);
  result.success = true;
  return true;
}

bool tryPackMsdfAtlas(const stbtt_fontinfo &fontInfo, int pointSize,
                      int sheetSize, const Array<int> &codepoints,
                      int msdfPixelRange, PackedAtlasData &result) {
  result.codepoints.clear();
  result.packedChars.clear();
  result.glyphLookup.clear();
  result.atlasBitmap.clear();

  const float scale =
      stbtt_ScaleForPixelHeight(&fontInfo, static_cast<float>(pointSize));
  result.codepoints = codepoints;
  result.packedChars.resize(result.codepoints.size());

  Array<stbrp_rect> rects(result.codepoints.size());
  Array<int> glyphs(result.codepoints.size());
  Array<int> x0s(result.codepoints.size());
  Array<int> y0s(result.codepoints.size());
  Array<int> x1s(result.codepoints.size());
  Array<int> y1s(result.codepoints.size());

  for (UInt32 i = 0; i < result.codepoints.size(); ++i) {
    const int glyph = stbtt_FindGlyphIndex(&fontInfo, result.codepoints[i]);
    glyphs[i] = glyph;
    stbtt_GetGlyphBitmapBoxSubpixel(&fontInfo, glyph, scale, scale, 0.0f, 0.0f,
                                    &x0s[i], &y0s[i], &x1s[i], &y1s[i]);
    const int glyphWidth = std::max(0, x1s[i] - x0s[i]);
    const int glyphHeight = std::max(0, y1s[i] - y0s[i]);
    rects[i].id = static_cast<int>(i);
    rects[i].w = glyphWidth > 0
                     ? glyphWidth + msdfPixelRange * 2 + kMsdfAtlasGutter * 2
                     : 0;
    rects[i].h = glyphHeight > 0
                     ? glyphHeight + msdfPixelRange * 2 + kMsdfAtlasGutter * 2
                     : 0;
    rects[i].x = 0;
    rects[i].y = 0;
    rects[i].was_packed = 0;
  }

  stbtt_pack_context spc = {};
  if (!stbtt_PackBegin(&spc, nullptr, sheetSize, sheetSize, 0, 1, nullptr)) {
    return false;
  }
  stbtt_PackFontRangesPackRects(&spc, rects.data(),
                                static_cast<int>(rects.size()));
  stbtt_PackEnd(&spc);

  Array<unsigned char> atlas(
      static_cast<size_t>(sheetSize) * static_cast<size_t>(sheetSize) * 4u, 0);

  for (UInt32 i = 0; i < result.codepoints.size(); ++i) {
    const auto &rect = rects[i];
    if (rect.w > 0 && rect.h > 0 && !rect.was_packed) {
      return false;
    }

    int advance = 0;
    int lsb = 0;
    stbtt_GetGlyphHMetrics(&fontInfo, glyphs[i], &advance, &lsb);

    stbtt_packedchar &packed = result.packedChars[i];
    packed.x0 = static_cast<unsigned short>(rect.x);
    packed.y0 = static_cast<unsigned short>(rect.y);
    packed.x1 = static_cast<unsigned short>(rect.x + rect.w);
    packed.y1 = static_cast<unsigned short>(rect.y + rect.h);
    packed.xoff =
        static_cast<float>(x0s[i] - msdfPixelRange - kMsdfAtlasGutter);
    packed.yoff =
        static_cast<float>(y0s[i] - msdfPixelRange - kMsdfAtlasGutter);
    packed.xoff2 = packed.xoff + static_cast<float>(rect.w);
    packed.yoff2 = packed.yoff + static_cast<float>(rect.h);
    packed.xadvance = static_cast<float>(advance) * scale;
  }

  std::atomic<UInt32> nextGlyph{0};
  std::atomic<bool> failed{false};
  std::exception_ptr workerException;
  std::mutex exceptionMutex;

  auto renderGlyphs = [&]() {
    try {
      while (!failed.load(std::memory_order_relaxed)) {
        const UInt32 i = nextGlyph.fetch_add(1, std::memory_order_relaxed);
        if (i >= result.codepoints.size()) {
          break;
        }

        const auto &rect = rects[i];
        if (rect.w == 0 || rect.h == 0) {
          continue;
        }

        msdfgen::Shape shape = makeGlyphShape(fontInfo, glyphs[i], scale);
        if (shape.contours.empty()) {
          continue;
        }

        msdfgen::Bitmap<float, 4> msdf(rect.w, rect.h);
        const msdfgen::Vector2 msdfScale(1.0, 1.0);
        const msdfgen::Vector2 msdfTranslate(
            -static_cast<double>(x0s[i]) + msdfPixelRange + kMsdfAtlasGutter,
            -static_cast<double>(y0s[i]) + msdfPixelRange + kMsdfAtlasGutter);
        const msdfgen::Range msdfRange(msdfPixelRange);
        const msdfgen::Projection msdfProjection(msdfScale, msdfTranslate);
        msdfgen::generateMTSDF_legacy(msdf, shape, msdfRange, msdfScale,
                                      msdfTranslate);
        msdfgen::distanceSignCorrection(msdf, shape, msdfScale, msdfTranslate);
        msdfgen::msdfErrorCorrection(msdf, shape, msdfProjection, msdfRange,
                                     msdfgen::MSDFGeneratorConfig(false));

        for (int y = 0; y < rect.h; ++y) {
          for (int x = 0; x < rect.w; ++x) {
            const float *src = msdf(x, y);
            const size_t dst =
                (static_cast<size_t>(rect.y + y) * sheetSize + rect.x + x) * 4u;
            atlas[dst + 0] = msdfFloatToByte(src[0]);
            atlas[dst + 1] = msdfFloatToByte(src[1]);
            atlas[dst + 2] = msdfFloatToByte(src[2]);
            atlas[dst + 3] = msdfFloatToByte(src[3]);
          }
        }
      }
    } catch (...) {
      failed.store(true, std::memory_order_relaxed);
      std::lock_guard<std::mutex> lock(exceptionMutex);
      if (!workerException) {
        workerException = std::current_exception();
      }
    }
  };

  const std::size_t workerCount = msdfWorkerCount(result.codepoints.size());
  Array<std::thread> workers;
  workers.reserve(workerCount > 0 ? workerCount - 1 : 0);
  for (std::size_t i = 1; i < workerCount; ++i) {
    workers.emplace_back(renderGlyphs);
  }
  renderGlyphs();
  for (auto &worker : workers) {
    worker.join();
  }
  if (workerException) {
    std::rethrow_exception(workerException);
  }
  if (failed.load(std::memory_order_relaxed)) {
    return false;
  }

  result.atlasBitmap = std::move(atlas);
  result.textureFormat = gpu::TextureFormat::R8G8B8A8_UNORM;
  result.channels = 4;
  result.distanceFieldRange = msdfPixelRange;
  result.glyphLookup.reserve(result.codepoints.size());
  for (UInt32 i = 0; i < result.codepoints.size(); ++i) {
    result.glyphLookup.emplace(static_cast<UInt32>(result.codepoints[i]), i);
  }
  result.sheetSize = static_cast<UInt32>(sheetSize);
  result.success = true;
  return true;
}

} // namespace

class FontImpl : public Font {
private:
  Array<stbtt_packedchar> packedChars;
  Array<unsigned char> fontBytes;
  Hashmap<UInt32, UInt32> glyphLookup;
  stbtt_fontinfo fontInfo;
  Ptr<Texture> texture;
  UInt32 sheetSize;
  UInt32 fallbackGlyphIndex;
  FontMethod method;
  int distanceFieldRange;
  mutable Hashmap<String, TextDrawData> textCache;
  std::atomic<bool> loaded = false;
  int m_size;

public:
  FontImpl()
      : packedChars(), fontBytes(), glyphLookup(), texture(), sheetSize(0),
        fallbackGlyphIndex(0), method(FontMethod::Bitmap),
        distanceFieldRange(1), textCache(), loaded(false), m_size(0) {
    texture = Texture::create();
  }
  FontImpl(int32_t point, StringView file_name,
           FontMethod fontMethod = FontMethod::MSDF)
      : FontImpl() {
    load(point, file_name, fontMethod);
  }
  ~FontImpl() {}

  bool loadFromBytes(int pointSize, Array<unsigned char> &&bytes,
                     FontMethod fontMethod) {
    const int bakedPointSize = fontMethod == FontMethod::MSDF
                                   ? std::max(1, pointSize)
                                   : std::max(1, pointSize + 16);
    this->loaded = false;
    this->m_size = bakedPointSize;
    this->sheetSize = 0;
    this->fallbackGlyphIndex = 0;
    this->method = fontMethod;
    this->distanceFieldRange = fontMethod == FontMethod::MSDF
                                   ? msdfPixelRangeForSize(bakedPointSize)
                                   : 1;
    this->textCache.clear();
    this->fontBytes = std::move(bytes);

    const int fontOffset =
        this->fontBytes.empty()
            ? -1
            : stbtt_GetFontOffsetForIndex(this->fontBytes.data(), 0);
    if (fontOffset < 0 ||
        !stbtt_InitFont(&fontInfo, this->fontBytes.data(), fontOffset)) {
      return false;
    }

    this->loaded = true;
    return true;
  }

  bool load(int pointSize, FontMethod fontMethod) override {
    Array<unsigned char> bytes(mplus1pMediumTtf,
                               mplus1pMediumTtf + mplus1pMediumTtfLen);
    return loadFromBytes(pointSize, std::move(bytes), fontMethod);
  }

  bool load(int pointSize, StringView fontName,
            FontMethod fontMethod) override {
    const String data = AssetReader::readAsString(fontName);
    Array<unsigned char> bytes(data.begin(), data.end());
    return loadFromBytes(pointSize, std::move(bytes), fontMethod);
  }

  bool load(int pointSize, const Buffer &buffer,
            FontMethod fontMethod) override {
    Array<unsigned char> bytes(buffer.size());
    std::memcpy(bytes.data(), buffer.data(),
                static_cast<size_t>(buffer.size()));
    return loadFromBytes(pointSize, std::move(bytes), fontMethod);
  }

  bool isLoaded() override { return loaded; }
  void unload() override {}
  int size() const override { return m_size; }

  void resize(int point_size) override {}
  Ptr<Texture> getAtlas() const override { return this->texture; }

  static const char *utf8ToCodepoint(const char *p, uint32_t *out_cp) {
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

  typedef struct {
    float w, h;
    float minx, miny, maxx, maxy;
  } TextBounds;

  TextBounds measureTextUTF32(const stbtt_fontinfo *font, float pixelHeight,
                              const UInt32 *cps, int count) {
    float scale = stbtt_ScaleForPixelHeight(font, pixelHeight);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(font, &ascent, &descent, &lineGap);
    float baseline = ascent * scale;

    float x = 0.0f;
    float minx = 1e30f, miny = 1e30f;
    float maxx = -1e30f, maxy = -1e30f;

    int prev = 0;
    for (int i = 0; i < count; i++) {
      int cp = cps[i];

      // kerning
      if (prev)
        x += stbtt_GetCodepointKernAdvance(font, prev, cp) * scale;

      int advance, lsb;
      stbtt_GetCodepointHMetrics(font, cp, &advance, &lsb);

      int x0, y0, x1, y1;
      stbtt_GetCodepointBitmapBox(font, cp, scale, scale, &x0, &y0, &x1, &y1);

      float gx0 = x + x0;
      float gy0 = baseline + y0;
      float gx1 = x + x1;
      float gy1 = baseline + y1;

      if (gx0 < minx)
        minx = gx0;
      if (gy0 < miny)
        miny = gy0;
      if (gx1 > maxx)
        maxx = gx1;
      if (gy1 > maxy)
        maxy = gy1;

      // pen advance
      x += advance * scale;
      prev = cp;
    }

    TextBounds b;
    b.minx = (count ? minx : 0.0f);
    b.miny = (count ? miny : 0.0f);
    b.maxx = (count ? maxx : 0.0f);
    b.maxy = (count ? maxy : 0.0f);
    b.w = (count ? (maxx - minx) : 0.0f);
    b.h = (count ? (maxy - miny) : 0.0f);
    return b;
  }

  Rect region(StringView text, int fontSize, const Pivot &pivot,
              const Vec2 &vec) override {
    const char *p = text.data();
    Array<UInt32> cps;
    while (*p) {
      UInt32 cp;
      p = utf8ToCodepoint(p, &cp);
      cps.push_back(cp);
    }
    auto m = measureTextUTF32(&fontInfo, fontSize, cps.data(), cps.size());
    return Rect(pivot, vec.x, vec.y, m.w * 2.f, m.h * 2.f);
  }

  Array<int> collectTextCodepoints(StringView text) const {
    Array<int> codepoints;
    Hashmap<UInt32, UInt32> seen;
    const char *p = text.data();
    while (*p) {
      UInt32 cp = 0;
      p = utf8ToCodepoint(p, &cp);
      if (stbtt_FindGlyphIndex(&fontInfo, static_cast<int>(cp)) == 0) {
        if (stbtt_FindGlyphIndex(
                &fontInfo, static_cast<int>(kFallbackAsciiCodepoint)) != 0) {
          cp = kFallbackAsciiCodepoint;
        } else if (stbtt_FindGlyphIndex(
                       &fontInfo, static_cast<int>(kReplacementCodepoint)) !=
                   0) {
          cp = kReplacementCodepoint;
        } else {
          continue;
        }
      }
      if (seen.emplace(cp, 1).second) {
        codepoints.push_back(static_cast<int>(cp));
      }
    }
    return codepoints;
  }

  Mesh makeTextMesh(StringView text, const Array<stbtt_packedchar> &chars,
                    const Hashmap<UInt32, UInt32> &lookup, UInt32 fallbackIndex,
                    UInt32 atlasSize) const {
    auto textMesh = makePtr<Mesh::Data>();
    if (chars.empty() || atlasSize == 0) {
      return Mesh{textMesh};
    }

    float x = 0.f, y = 0.f;
    Vec2 yrange(Math::infinity, Math::negInfinity);
    const char *p = text.data();
    while (*p) {
      uint32_t cp;
      const auto *next = utf8ToCodepoint(p, &cp);
      stbtt_aligned_quad q;
      UInt32 glyphIndex = fallbackIndex;
      if (auto it = lookup.find(cp); it != lookup.end()) {
        glyphIndex = it->second;
      }

      stbtt_GetPackedQuad(chars.data(), atlasSize, atlasSize,
                          static_cast<int>(glyphIndex), &x, &y, &q, 1);
      UInt32 startIndex = textMesh->vertices.size();
      auto &vertices = textMesh->vertices;
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

      textMesh->indices.push_back(startIndex + 0);
      textMesh->indices.push_back(startIndex + 1);
      textMesh->indices.push_back(startIndex + 2);
      textMesh->indices.push_back(startIndex + 2);
      textMesh->indices.push_back(startIndex + 1);
      textMesh->indices.push_back(startIndex + 3);
      p = next;
    }
    if (textMesh->vertices.empty()) {
      return Mesh{textMesh};
    }
    for (auto &v : textMesh->vertices) {
      v.position.x -= x * 2.f;
      v.position.y -= (yrange.y - yrange.x) / 2.f;
    }
    return Mesh{textMesh};
  }

  Mesh getTextMesh(StringView text) const override {
    return makeTextDrawData(text).mesh;
  }

  TextDrawData makeTextDrawData(StringView text) const override {
    TextDrawData data;
    if (!this->loaded.load()) {
      return data;
    }

    const String cacheKey(text.data(), text.size());
    if (auto it = textCache.find(cacheKey); it != textCache.end()) {
      return it->second;
    }

    const Array<int> codepoints = collectTextCodepoints(text);
    if (codepoints.empty()) {
      return data;
    }

    PackedAtlasData atlasData;
    const int initialSheetSize =
        estimateInitialAtlasSize(fontInfo, this->m_size, codepoints,
                                 this->method, this->distanceFieldRange);
    for (int sheetSize : kAtlasSizes) {
      if (sheetSize < initialSheetSize) {
        continue;
      }
      const bool packed =
          this->method == FontMethod::MSDF
              ? tryPackMsdfAtlas(fontInfo, this->m_size, sheetSize, codepoints,
                                 this->distanceFieldRange, atlasData)
              : tryPackAtlas(this->fontBytes.data(), this->m_size, sheetSize,
                             codepoints, atlasData);
      if (packed) {
        break;
      }
    }
    if (!atlasData.success || atlasData.sheetSize == 0 ||
        atlasData.packedChars.empty()) {
      return data;
    }

    Ptr<Texture> textTexture = Texture::create();
    textTexture->loadFromMemory(atlasData.atlasBitmap.data(),
                                atlasData.sheetSize, atlasData.sheetSize,
                                atlasData.textureFormat, atlasData.channels);

    data.mesh = makeTextMesh(text, atlasData.packedChars, atlasData.glyphLookup,
                             selectFallbackGlyphIndex(atlasData.glyphLookup),
                             atlasData.sheetSize);
    data.texture = textTexture;
    data.distanceFieldRange = static_cast<float>(atlasData.distanceFieldRange);
    data.valid = true;
    textCache.emplace(cacheKey, data);
    return data;
  }
};
Ptr<Font> Font::create() { return makePtr<FontImpl>(); }
Ptr<Font> Font::create(int32_t point, StringView fileName) {
  return makePtr<FontImpl>(point, fileName);
}
Ptr<Font> Font::create(int32_t point, StringView fileName, FontMethod method) {
  return makePtr<FontImpl>(point, fileName, method);
}
} // namespace sinen

