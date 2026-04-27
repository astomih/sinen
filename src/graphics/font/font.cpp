// std
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <future>
#include <memory>

// internal
#include <core/data/array.hpp>
#include <core/data/hashmap.hpp>
#include <gpu/gpu.hpp>
#include <graphics/font/font.hpp>
#include <graphics/font/font_glyph_ranges.hpp>
#include <graphics/graphics.hpp>
#include <graphics/texture/texture.hpp>
#include <math/color/color.hpp>
#include <math/geometry/mesh.hpp>
#include <math/math.hpp>
#include <platform/io/asset_io.hpp>

// thread
#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>

// external
#include <SDL3/SDL.h>
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_RASTERIZER_VERSION 2
#include <stb_truetype.h>

#include "default/mplus-1p-medium.ttf.hpp"

namespace sinen {
namespace {

constexpr UInt32 kFallbackAsciiCodepoint = '?';
constexpr UInt32 kReplacementCodepoint = 0xFFFD;
constexpr int kAtlasSizes[] = {1024, 2048, 4096, 8192};

struct PackedAtlasData {
  Array<int> codepoints;
  Array<stbtt_packedchar> packedChars;
  Hashmap<UInt32, UInt32> glyphLookup;
  Array<unsigned char> atlasBitmap;
  UInt32 sheetSize = 0;
  bool success = false;
};

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

bool tryPackAtlas(const unsigned char *fontData, int pointSize, int sheetSize,
                  const Array<UInt32> &sourceCodepoints,
                  PackedAtlasData &result) {
  result.codepoints.clear();
  result.packedChars.clear();
  result.glyphLookup.clear();
  result.atlasBitmap.clear();

  result.codepoints.reserve(sourceCodepoints.size());
  for (UInt32 cp : sourceCodepoints) {
    result.codepoints.push_back(static_cast<int>(cp));
  }

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
  stbtt_PackSetSkipMissingCodepoints(&spc, 0);
  const int packed = stbtt_PackFontRanges(&spc, fontData, 0, &range, 1);
  stbtt_PackEnd(&spc);
  if (!packed) {
    return false;
  }

  result.atlasBitmap.resize(sheetSize * sheetSize * 4);
  for (int y = 0; y < sheetSize; ++y) {
    for (int x = 0; x < sheetSize; ++x) {
      const int grayIndex = y * sheetSize + x;
      const int rgbaIndex = grayIndex * 4;
      const unsigned char alpha = monoAtlas[grayIndex];
      result.atlasBitmap[rgbaIndex + 0] = 255;
      result.atlasBitmap[rgbaIndex + 1] = 255;
      result.atlasBitmap[rgbaIndex + 2] = 255;
      result.atlasBitmap[rgbaIndex + 3] = alpha;
    }
  }

  result.glyphLookup.reserve(result.codepoints.size());
  for (UInt32 i = 0; i < result.codepoints.size(); ++i) {
    result.glyphLookup.emplace(static_cast<UInt32>(result.codepoints[i]), i);
  }

  result.sheetSize = static_cast<UInt32>(sheetSize);
  result.success = true;
  return true;
}

PackedAtlasData loadCore(const unsigned char *fontData, int pointSize) {
  PackedAtlasData result;
  const auto &codepoints = font::defaultGlyphCodepoints();
  for (int sheetSize : kAtlasSizes) {
    if (tryPackAtlas(fontData, pointSize, sheetSize, codepoints, result)) {
      return result;
    }
  }
  result.success = false;
  return result;
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
  std::future<PackedAtlasData> future;
  Array<unsigned char> atlasBitmap;
  std::atomic<bool> loaded = false;
  int m_size;

public:
  FontImpl()
      : packedChars(), fontBytes(), glyphLookup(), texture(), sheetSize(0),
        fallbackGlyphIndex(0), future(), atlasBitmap(), loaded(false),
        m_size(0) {
    texture = Texture::create();
  }
  FontImpl(int32_t point, StringView file_name) : FontImpl() {
    load(point, file_name);
  }
  ~FontImpl() {}

  bool loadFromBytes(int pointSize, Array<unsigned char> &&bytes) {
    pointSize += 16;
    this->loaded = false;
    this->m_size = pointSize;
    this->sheetSize = 0;
    this->fallbackGlyphIndex = 0;
    this->fontBytes = std::move(bytes);

    if (this->fontBytes.empty() ||
        !stbtt_InitFont(&fontInfo, this->fontBytes.data(), 0)) {
      return false;
    }

    const TaskGroup group = LoadContext::current();
    group.add();

    this->future =
        globalThreadPool().submit(loadCore, this->fontBytes.data(), pointSize);

    auto pollAndUpload = std::make_shared<std::function<void()>>();
    *pollAndUpload = [this, pollAndUpload, group]() {
      if (this->loaded) {
        return;
      }
      if (!this->future.valid()) {
        group.done();
        return;
      }

      if (this->future.wait_for(std::chrono::milliseconds(0)) !=
          std::future_status::ready) {
        Graphics::addPreDrawFunc(*pollAndUpload);
        return;
      }

      PackedAtlasData atlasData = this->future.get();
      if (!atlasData.success || atlasData.sheetSize == 0 ||
          atlasData.packedChars.empty()) {
        group.done();
        return;
      }

      this->packedChars = std::move(atlasData.packedChars);
      this->glyphLookup = std::move(atlasData.glyphLookup);
      this->atlasBitmap = std::move(atlasData.atlasBitmap);
      this->sheetSize = atlasData.sheetSize;
      this->fallbackGlyphIndex = selectFallbackGlyphIndex(this->glyphLookup);

      this->texture->loadFromMemory(this->atlasBitmap.data(), this->sheetSize,
                                    this->sheetSize,
                                    gpu::TextureFormat::R8G8B8A8_UNORM, 4);
      this->atlasBitmap.clear();
      this->atlasBitmap.shrink_to_fit();
      this->loaded = true;
      group.done();
    };
    Graphics::addPreDrawFunc(*pollAndUpload);
    return true;
  }

  bool load(int pointSize) override {
    Array<unsigned char> bytes(mplus1pMediumTtf,
                               mplus1pMediumTtf + mplus1pMediumTtfLen);
    return loadFromBytes(pointSize, std::move(bytes));
  }

  bool load(int pointSize, StringView fontName) override {
    const String data = AssetIO::openAsString(fontName);
    Array<unsigned char> bytes(data.begin(), data.end());
    return loadFromBytes(pointSize, std::move(bytes));
  }

  bool load(int pointSize, const Buffer &buffer) override {
    Array<unsigned char> bytes(buffer.size());
    std::memcpy(bytes.data(), buffer.data(),
                static_cast<size_t>(buffer.size()));
    return loadFromBytes(pointSize, std::move(bytes));
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

  Mesh getTextMesh(StringView text) const override {

    auto textMesh = makePtr<Mesh::Data>();
    if (this->packedChars.empty() || this->sheetSize == 0) {
      return Mesh{textMesh};
    }

    float x = 0.f, y = 0.f;
    Vec2 yrange(Math::infinity, Math::negInfinity);
    const char *p = text.data();
    while (*p) {
      uint32_t cp;
      const auto *next = utf8ToCodepoint(p, &cp);
      stbtt_aligned_quad q;
      UInt32 glyphIndex = fallbackGlyphIndex;
      if (auto it = glyphLookup.find(cp); it != glyphLookup.end()) {
        glyphIndex = it->second;
      }

      const UInt32 atlasSize = this->sheetSize;
      stbtt_GetPackedQuad(this->packedChars.data(), atlasSize, atlasSize,
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
};
Ptr<Font> Font::create() { return makePtr<FontImpl>(); }
Ptr<Font> Font::create(int32_t point, StringView fileName) {
  return makePtr<FontImpl>(point, fileName);
}
} // namespace sinen

#include <script/luaapi.hpp>
namespace sinen {

static int lFontNew(lua_State *L) {
  udPushPtr<Font>(L, Font::create());
  return 1;
}
static int lFontLoad(lua_State *L) {
  auto &font = udPtr<Font>(L, 1);
  int n = lua_gettop(L);
  int point = static_cast<int>(luaL_checkinteger(L, 2));
  if (n == 2) {
    lua_pushboolean(L, font->load(point));
    return 1;
  }
  if (lua_isstring(L, 3)) {
    const char *path = luaL_checkstring(L, 3);
    lua_pushboolean(L, font->load(point, StringView(path)));
    return 1;
  }
  auto &buf = udValue<Buffer>(L, 3);
  lua_pushboolean(L, font->load(point, buf));
  return 1;
}
static int lFontResize(lua_State *L) {
  auto &font = udPtr<Font>(L, 1);
  int point = static_cast<int>(luaL_checkinteger(L, 2));
  font->resize(point);
  return 0;
}
static int lFontRegion(lua_State *L) {
  auto &font = udPtr<Font>(L, 1);
  const char *text = luaL_checkstring(L, 2);
  int fontSize = static_cast<int>(luaL_checkinteger(L, 3));
  Pivot pivot = static_cast<Pivot>(luaL_checkinteger(L, 4));
  auto &vec = udValue<Vec2>(L, 5);
  udNewOwned<Rect>(L, font->region(StringView(text), fontSize, pivot, vec));
  return 1;
}
void registerFont(lua_State *L) {
  luaL_newmetatable(L, Font::metaTableName());
  luaPushcfunction2(L, udPtrGc<Font>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lFontLoad);
  lua_setfield(L, -2, "load");
  luaPushcfunction2(L, lFontResize);
  lua_setfield(L, -2, "resize");
  luaPushcfunction2(L, lFontRegion);
  lua_setfield(L, -2, "region");
  lua_pop(L, 1);

  pushSnNamed(L, "Font");
  luaPushcfunction2(L, lFontNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

} // namespace sinen
