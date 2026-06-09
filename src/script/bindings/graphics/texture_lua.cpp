#include <core/profiler.hpp>
#include <core/thread/future_poll.hpp>
#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>
#include <gpu/gpu.hpp>
#include <graphics/graphics.hpp>
#include <graphics/texture/texture.hpp>
#include <math/math.hpp>
#include <platform/io/asset_reader.hpp>
#include <script/luaapi.hpp>

namespace sinen {
static int lTextureNew(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udPushPtr<Texture>(L, Texture::create());
    return 1;
  }
  int w = static_cast<int>(luaL_checkinteger(L, 1));
  int h = static_cast<int>(luaL_checkinteger(L, 2));
  udPushPtr<Texture>(L, Texture::create(w, h));
  return 1;
}
static int lTextureLoad(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  if (lua_isstring(L, 2)) {
    const char *path = luaL_checkstring(L, 2);
    tex->load(StringView(path));
    return 0;
  }
  auto &buf = udValue<Buffer>(L, 2);
  tex->load(buf);
  return 0;
}
static int lTextureLoadCubemap(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  const char *path = luaL_checkstring(L, 2);
  tex->loadCubemap(StringView(path));
  return 0;
}
static int lTextureLoadIrradianceCubemap(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  const char *path = luaL_checkstring(L, 2);
  const auto faceSizeArg = luaL_optinteger(L, 3, 64);
  const auto sampleCountArg = luaL_optinteger(L, 4, 256);
  if (faceSizeArg <= 0 || sampleCountArg <= 0) {
    lua_pushboolean(L, false);
    return 1;
  }
  lua_pushboolean(L, tex->loadIrradianceCubemap(
                         StringView(path), static_cast<uint32_t>(faceSizeArg),
                         static_cast<uint32_t>(sampleCountArg)));
  return 1;
}
static int lTextureLoadPrefilteredCubemap(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  const char *path = luaL_checkstring(L, 2);
  const auto faceSizeArg = luaL_optinteger(L, 3, 128);
  const auto mipLevelsArg = luaL_optinteger(L, 4, 5);
  const auto sampleCountArg = luaL_optinteger(L, 5, 128);
  if (faceSizeArg <= 0 || mipLevelsArg <= 0 || sampleCountArg <= 0) {
    lua_pushboolean(L, false);
    return 1;
  }
  lua_pushboolean(L, tex->loadPrefilteredCubemap(
                         StringView(path), static_cast<uint32_t>(faceSizeArg),
                         static_cast<uint32_t>(mipLevelsArg),
                         static_cast<uint32_t>(sampleCountArg)));
  return 1;
}
static int lTextureLoadBRDFLUT(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  const auto sizeArg = luaL_optinteger(L, 2, 256);
  const auto sampleCountArg = luaL_optinteger(L, 3, 512);
  if (sizeArg <= 0 || sampleCountArg <= 0) {
    lua_pushboolean(L, false);
    return 1;
  }
  lua_pushboolean(L, tex->loadBRDFLUT(static_cast<uint32_t>(sizeArg),
                                      static_cast<uint32_t>(sampleCountArg)));
  return 1;
}
static int lTextureLoadPixels(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  auto &buf = udValue<Buffer>(L, 2);
  const auto width = static_cast<uint32_t>(luaL_checkinteger(L, 3));
  const auto height = static_cast<uint32_t>(luaL_checkinteger(L, 4));
  const char *formatName = luaL_optstring(L, 5, "rgba8");

  gpu::TextureFormat format = gpu::TextureFormat::R8G8B8A8_UNORM;
  int channels = 4;
  if (StringView(formatName) == "bgra8" || StringView(formatName) == "BGRA8") {
    format = gpu::TextureFormat::B8G8R8A8_UNORM;
  }

  lua_pushboolean(L, tex->loadPixels(buf, width, height, format, channels));
  return 1;
}
static int lTextureFill(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  auto &c = udValue<Color>(L, 2);
  tex->fill(c);
  return 0;
}
static int lTextureCopy(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  udPushPtr<Texture>(L, tex->copy());
  return 1;
}
static int lTextureSize(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  udNewOwned<Vec2>(L, tex->size());
  return 1;
}
static int lTextureTostring(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  String s = tex->tableString();
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
void registerTexture(lua_State *L) {
  luaL_newmetatable(L, Texture::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lTextureLoad);
  lua_setfield(L, -2, "load");
  luaPushcfunction2(L, lTextureLoadCubemap);
  lua_setfield(L, -2, "loadCubemap");
  luaPushcfunction2(L, lTextureLoadIrradianceCubemap);
  lua_setfield(L, -2, "loadIrradianceCubemap");
  luaPushcfunction2(L, lTextureLoadPrefilteredCubemap);
  lua_setfield(L, -2, "loadPrefilteredCubemap");
  luaPushcfunction2(L, lTextureLoadBRDFLUT);
  lua_setfield(L, -2, "loadBRDFLUT");
  luaPushcfunction2(L, lTextureLoadPixels);
  lua_setfield(L, -2, "loadPixels");
  luaPushcfunction2(L, lTextureFill);
  lua_setfield(L, -2, "fill");
  luaPushcfunction2(L, lTextureCopy);
  lua_setfield(L, -2, "copy");
  luaPushcfunction2(L, lTextureSize);
  lua_setfield(L, -2, "size");
  luaPushcfunction2(L, lTextureTostring);
  lua_setfield(L, -2, "__tostring");
  lua_pop(L, 1);

  pushSnNamed(L, "Texture");
  luaPushcfunction2(L, lTextureNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
