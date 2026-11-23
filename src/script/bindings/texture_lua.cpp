#include "luaapi.hpp"
#include <core/profiler.hpp>
#include <core/thread/future_poll.hpp>
#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>
#include <gpu/gpu.hpp>
#include <graphics/graphics.hpp>
#include <graphics/texture/texture.hpp>
#include <math/math.hpp>
#include <platform/io/asset_reader.hpp>
#include <platform/io/external_file.hpp>

namespace sinen {
static int lTextureNew(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 1) {
    auto texture = Texture::create();
    if (lua_isstring(L, 1)) {
      const char *path = luaL_checkstring(L, 1);
      if (!AssetReader::exists(path)) {
        texture.reset();
        return luaLError2(L, "sn.Texture.new asset not found: %s", path);
      }
      texture->load(StringView(path));
    } else if (auto *file = udValueOrNull<ExternalFile>(L, 1)) {
      auto buffer = file->read();
      if (!buffer || buffer->size() == 0) {
        texture.reset();
        return luaLError2(
            L, "sn.Texture.new external file is empty or unreadable");
      }
      texture->load(*buffer);
    } else {
      auto &buffer = udValue<Buffer>(L, 1);
      if (buffer.size() == 0) {
        texture.reset();
        return luaLError2(L, "sn.Texture.new source buffer is empty");
      }
      texture->load(buffer);
    }
    udPushPtr<Texture>(L, std::move(texture));
    return 1;
  }
  if (n != 2) {
    return luaLError2(L, "sn.Texture.new expects a source or width and height");
  }
  int w = static_cast<int>(luaL_checkinteger(L, 1));
  int h = static_cast<int>(luaL_checkinteger(L, 2));
  if (w <= 0 || h <= 0) {
    return luaLError2(L, "sn.Texture.new dimensions must be greater than zero");
  }
  udPushPtr<Texture>(L, Texture::create(w, h));
  return 1;
}
static int lTextureNewCubemap(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  if (!AssetReader::exists(path)) {
    return luaLError2(L, "sn.Texture.newCubemap asset not found: %s", path);
  }
  auto texture = Texture::create();
  texture->loadCubemap(StringView(path));
  udPushPtr<Texture>(L, std::move(texture));
  return 1;
}
static int lTextureNewIrradianceCubemap(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const auto faceSizeArg = luaL_optinteger(L, 2, 64);
  const auto sampleCountArg = luaL_optinteger(L, 3, 256);
  if (faceSizeArg <= 0 || sampleCountArg <= 0) {
    return luaLError2(
        L,
        "sn.Texture.newIrradianceCubemap arguments must be greater than zero");
  }
  if (!AssetReader::exists(path)) {
    return luaLError2(L, "sn.Texture.newIrradianceCubemap asset not found: %s",
                      path);
  }
  auto texture = Texture::create();
  texture->loadIrradianceCubemap(StringView(path),
                                 static_cast<uint32_t>(faceSizeArg),
                                 static_cast<uint32_t>(sampleCountArg));
  udPushPtr<Texture>(L, std::move(texture));
  return 1;
}
static int lTextureNewPrefilteredCubemap(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const auto faceSizeArg = luaL_optinteger(L, 2, 128);
  const auto mipLevelsArg = luaL_optinteger(L, 3, 5);
  const auto sampleCountArg = luaL_optinteger(L, 4, 128);
  if (faceSizeArg <= 0 || mipLevelsArg <= 0 || sampleCountArg <= 0) {
    return luaLError2(
        L,
        "sn.Texture.newPrefilteredCubemap arguments must be greater than zero");
  }
  if (!AssetReader::exists(path)) {
    return luaLError2(L, "sn.Texture.newPrefilteredCubemap asset not found: %s",
                      path);
  }
  auto texture = Texture::create();
  texture->loadPrefilteredCubemap(StringView(path),
                                  static_cast<uint32_t>(faceSizeArg),
                                  static_cast<uint32_t>(mipLevelsArg),
                                  static_cast<uint32_t>(sampleCountArg));
  udPushPtr<Texture>(L, std::move(texture));
  return 1;
}
static int lTextureNewBRDFLUT(lua_State *L) {
  const auto sizeArg = luaL_optinteger(L, 1, 256);
  const auto sampleCountArg = luaL_optinteger(L, 2, 512);
  if (sizeArg <= 0 || sampleCountArg <= 0) {
    return luaLError2(
        L, "sn.Texture.newBRDFLUT arguments must be greater than zero");
  }
  auto texture = Texture::create();
  if (!texture->loadBRDFLUT(static_cast<uint32_t>(sizeArg),
                            static_cast<uint32_t>(sampleCountArg))) {
    texture.reset();
    return luaLError2(L, "sn.Texture.newBRDFLUT failed");
  }
  udPushPtr<Texture>(L, std::move(texture));
  return 1;
}
static int lTextureUpdatePixels(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  auto &buf = udValue<Buffer>(L, 2);
  const auto width = static_cast<uint32_t>(luaL_checkinteger(L, 3));
  const auto height = static_cast<uint32_t>(luaL_checkinteger(L, 4));
  const char *formatName = luaL_optstring(L, 5, "rgba8");

  gpu::TextureFormat format = gpu::TextureFormat::R8G8B8A8_UNORM;
  int channels = 4;
  if (StringView(formatName) == "r8" || StringView(formatName) == "R8") {
    format = gpu::TextureFormat::R8_UNORM;
    channels = 1;
  } else if (StringView(formatName) == "rg8" ||
             StringView(formatName) == "RG8") {
    format = gpu::TextureFormat::R8G8_UNORM;
    channels = 2;
  } else if (StringView(formatName) == "bgra8" ||
             StringView(formatName) == "BGRA8") {
    format = gpu::TextureFormat::B8G8R8A8_UNORM;
  }

  lua_pushboolean(L, tex->loadPixels(buf, width, height, format, channels));
  return 1;
}
static auto lTextureToPngBuffer(Ptr<Texture> &tex) {
  return tex->toPngBuffer();
}
static int lTextureToExrBuffer(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  const bool saveAsFp16 = lua_toboolean(L, 2);
  udNewOwned<Buffer>(L, tex->toExrBuffer(saveAsFp16));
  return 1;
}
static auto lTextureFill(Ptr<Texture> &tex, Color &c) {
  tex->fill(c);
  return;
}
static auto lTextureCopy(Ptr<Texture> &tex) { return tex->copy(); }
static auto lTextureSize(Ptr<Texture> &tex) { return tex->size(); }
static auto lTextureTostring(Ptr<Texture> &tex) {
  String s = tex->tableString();
  return s;
}
void registerTexture(luau::table &sn) {
  auto texture = engineClass<Ptr<Texture>>(sn, Texture::metaTableName());

  texture.raw_method<lTextureUpdatePixels>("updatePixels");
  texture.method<lTextureToPngBuffer>("toPngBuffer");
  texture.raw_method<lTextureToExrBuffer>("toExrBuffer");
  texture.method<lTextureFill>("fill");
  texture.method<lTextureCopy>("copy");
  texture.method<lTextureSize>("size");
  texture.metamethod<lTextureTostring>("__tostring");

  texture.raw_static_function<lTextureNew>("new");
  texture.raw_static_function<lTextureNewCubemap>("newCubemap");
  texture.raw_static_function<lTextureNewIrradianceCubemap>(
      "newIrradianceCubemap");
  texture.raw_static_function<lTextureNewPrefilteredCubemap>(
      "newPrefilteredCubemap");
  texture.raw_static_function<lTextureNewBRDFLUT>("newBRDFLUT");
}
} // namespace sinen
