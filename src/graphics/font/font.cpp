// std
#include <cassert>
#include <cstddef>

// internal
#include <core/data/array.hpp>
#include <gpu/gpu.hpp>
#include <graphics/font/font.hpp>
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

#include <chrono>
#include <future>
#include <memory>

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

class FontImpl : public Font {
private:
  Array<Array<stbtt_packedchar>> packedChar;
  stbtt_fontinfo fontInfo;
  Ptr<Texture> texture;
  uint32_t sheetSize;
  std::future<bool> future;
  Array<unsigned char> atlasBitmap;
  String data;
  std::atomic<bool> loaded = false;
  int m_size;

public:
  FontImpl() : packedChar(), texture(), sheetSize(0) {
    texture = Texture::create();
  }
  FontImpl(int32_t point, StringView file_name) { load(point, file_name); }
  ~FontImpl() {}
  static bool loadCore(const unsigned char *fontData,
                       Array<Array<stbtt_packedchar>> &pc,
                       Array<unsigned char> &atlasBitmap, int pointSize,
                       int sheetSize) {
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
    atlasBitmap.resize(sheetSize * sheetSize * 4);
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

    return true;
  }
  bool load(int pointSize) override {
    pointSize += 16;
    stbtt_InitFont(&fontInfo, mplus1pMediumTtf, 0);
    this->loaded = false;
    this->m_size = pointSize;
    this->sheetSize = pointSize * 64;

    const TaskGroup group = LoadContext::current();
    group.add();

    this->future = globalThreadPool().submit(
        loadCore, mplus1pMediumTtf, std::ref(this->packedChar),
        std::ref(this->atlasBitmap), pointSize, this->sheetSize);

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

      (void)this->future.get();
      this->texture->loadFromMemory(this->atlasBitmap.data(), this->sheetSize,
                                    this->sheetSize,
                                    gpu::TextureFormat::R8G8B8A8_UNORM, 4);
      this->loaded = true;
      group.done();
    };
    Graphics::addPreDrawFunc(*pollAndUpload);
    return true;
  }
  bool load(int pointSize, StringView fontName) override {
    pointSize += 16;
    this->loaded = false;
    this->m_size = pointSize;
    this->sheetSize = pointSize * 64;
    this->data = AssetIO::openAsString(fontName);
    const TaskGroup group = LoadContext::current();
    group.add();

    this->future = globalThreadPool().submit(
        loadCore, reinterpret_cast<const unsigned char *>(this->data.data()),
        std::ref(this->packedChar), std::ref(this->atlasBitmap), pointSize,
        this->sheetSize);

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

      (void)this->future.get();
      this->texture->loadFromMemory(this->atlasBitmap.data(), this->sheetSize,
                                    this->sheetSize,
                                    gpu::TextureFormat::R8G8B8A8_UNORM, 4);
      this->loaded = true;
      group.done();
    };
    Graphics::addPreDrawFunc(*pollAndUpload);
    return true;
  }
  bool load(int pointSize, const Buffer &buffer) override {
    pointSize += 16;
    this->loaded = false;
    this->m_size = pointSize;
    this->sheetSize = pointSize * 64;
    this->data = (const char *)buffer.data();
    const TaskGroup group = LoadContext::current();
    group.add();

    this->future = globalThreadPool().submit(
        loadCore, reinterpret_cast<const unsigned char *>(this->data.data()),
        std::ref(this->packedChar), std::ref(this->atlasBitmap), pointSize,
        this->sheetSize);

    auto pollAndUpload = makePtr<std::function<void()>>();
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

      (void)this->future.get();
      this->texture->loadFromMemory(this->atlasBitmap.data(), this->sheetSize,
                                    this->sheetSize,
                                    gpu::TextureFormat::R8G8B8A8_UNORM, 4);
      this->loaded = true;
      group.done();
    };
    Graphics::addPreDrawFunc(*pollAndUpload);
    return true;
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
    float x = 0.f, y = 0.f;
    Vec2 yrange(Math::infinity, Math::negInfinity);
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

      uint32_t sheetSize = this->sheetSize;
      stbtt_GetPackedQuad(this->packedChar[idx1].data(), sheetSize, sheetSize,
                          idx2, &x, &y, &q, 1);
      uint32_t startIndex = textMesh->vertices.size();
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
