#include <core/allocator/global_allocator.hpp>
#include <core/profiler.hpp>
#include <gpu/gpu.hpp>
#include <gpu/shader/builtin_shader.hpp>
#include <graphics/builtin_pipeline.hpp>
#include <graphics/font/default/mplus-1p-medium.ttf.hpp>
#include <graphics/graphics.hpp>
#include <graphics/gui/gui.hpp>
#include <graphics/texture/render_texture.hpp>
#include <math/transform/transform.hpp>
#include <platform/io/asset_reader.hpp>
#include <platform/window/window.hpp>
#include <script/luaapi.hpp>

namespace sinen {
static int lGraphicsDrawRect(lua_State *L) {
  auto &rect = udValue<Rect>(L, 1);
  auto &color = udValue<Color>(L, 2);
  if (lua_gettop(L) >= 3) {
    float angle = static_cast<float>(luaL_checknumber(L, 3));
    Graphics::drawRect(rect, color, angle);
    return 0;
  }
  Graphics::drawRect(rect, color);
  return 0;
}
static int lGraphicsDrawText(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  auto &style = udValue<TextStyle>(L, 2);
  auto &transform = udValue<TextTransform>(L, 3);
  Graphics::drawText(StringView(text), style, transform);
  return 0;
}
static int lGraphicsDrawImage(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  auto &rect = udValue<Rect>(L, 2);
  if (lua_gettop(L) >= 3) {
    float angle = static_cast<float>(luaL_checknumber(L, 3));
    Graphics::drawImage(tex, rect, angle);
    return 0;
  }
  Graphics::drawImage(tex, rect);
  return 0;
}
static int lGraphicsDrawCubemap(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  Graphics::drawCubemap(tex);
  return 0;
}
static int lGraphicsDrawModel(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  auto &t = udValue<Transform>(L, 2);
  Graphics::drawModel(*m, t);
  return 0;
}
static int lGraphicsDrawModelInstanced(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  luaL_checktype(L, 2, LUA_TTABLE);
  Array<Transform> transforms;
  size_t n = lua_objlen(L, 2);
  transforms.reserve(n);
  for (size_t i = 1; i <= n; ++i) {
    lua_rawgeti(L, 2, static_cast<lua_Integer>(i));
    transforms.push_back(udValue<Transform>(L, -1));
    lua_pop(L, 1);
  }
  Graphics::drawModelInstanced(*m, transforms);
  return 0;
}
static int lGraphicsBegin2D(lua_State *L) {
  if (lua_gettop(L) >= 1) {
    auto &cam = udValue<Camera2D>(L, 1);
    Graphics::begin2D(cam);
    return 0;
  }
  Graphics::begin2D();
  return 0;
}
static int lGraphicsBegin3D(lua_State *L) {
  auto &cam = udValue<Camera3D>(L, 1);
  Graphics::begin3D(cam);
  return 0;
}
static int lGraphicsFinish(lua_State *L) {
  (void)L;
  Graphics::finish();
  return 0;
}
static int lGraphicsGetClearColor(lua_State *L) {
  udNewOwned<Color>(L, Graphics::getClearColor());
  return 1;
}
static int lGraphicsSetClearColor(lua_State *L) {
  auto &c = udValue<Color>(L, 1);
  Graphics::setClearColor(c);
  return 0;
}
static int lGraphicsSetGraphicsPipeline(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  Graphics::setGraphicsPipeline(*p);
  return 0;
}
static int lGraphicsResetGraphicsPipeline(lua_State *L) {
  (void)L;
  Graphics::resetGraphicsPipeline();
  return 0;
}
static int lGraphicsSetTexture(lua_State *L) {
  UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  auto &t = udPtr<Texture>(L, 2);
  Graphics::setTexture(slot, t);
  return 0;
}
static int lGraphicsResetTexture(lua_State *L) {
  UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  Graphics::resetTexture(slot);
  return 0;
}
static int lGraphicsResetAllTexture(lua_State *L) {
  (void)L;
  Graphics::resetAllTexture();
  return 0;
}
static int lGraphicsSetUniformBuffer(lua_State *L) {
  UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  auto &b = udValue<Buffer>(L, 2);
  Graphics::setUniformBuffer(slot, b);
  return 0;
}
static int lGraphicsBeginRenderTarget(lua_State *L) {
  auto &rt = udPtr<RenderTexture>(L, 1);
  Graphics::beginRenderTarget(*rt);
  return 0;
}
static int lGraphicsEndRenderTarget(lua_State *L) {
  (void)L;
  Graphics::endRenderTarget();
  return 0;
}
static int lGraphicsReadbackTexture(lua_State *L) {
  auto &rt = udPtr<RenderTexture>(L, 1);
  auto &out = udPtr<Texture>(L, 2);
  lua_pushboolean(L, Graphics::readbackTexture(*rt, out));
  return 1;
}
void registerGraphics(lua_State *L) {
  pushSnNamed(L, "Graphics");
  luaPushcfunction2(L, lGraphicsDrawRect);
  lua_setfield(L, -2, "drawRect");
  luaPushcfunction2(L, lGraphicsDrawText);
  lua_setfield(L, -2, "drawText");
  luaPushcfunction2(L, lGraphicsDrawImage);
  lua_setfield(L, -2, "drawImage");
  luaPushcfunction2(L, lGraphicsDrawCubemap);
  lua_setfield(L, -2, "drawCubemap");
  luaPushcfunction2(L, lGraphicsDrawModel);
  lua_setfield(L, -2, "drawModel");
  luaPushcfunction2(L, lGraphicsDrawModelInstanced);
  lua_setfield(L, -2, "drawModelInstanced");
  luaPushcfunction2(L, lGraphicsBegin2D);
  lua_setfield(L, -2, "begin2D");
  luaPushcfunction2(L, lGraphicsBegin3D);
  lua_setfield(L, -2, "begin3D");
  luaPushcfunction2(L, lGraphicsFinish);
  lua_setfield(L, -2, "finish");
  luaPushcfunction2(L, lGraphicsGetClearColor);
  lua_setfield(L, -2, "getClearColor");
  luaPushcfunction2(L, lGraphicsSetClearColor);
  lua_setfield(L, -2, "setClearColor");
  luaPushcfunction2(L, lGraphicsSetGraphicsPipeline);
  lua_setfield(L, -2, "setGraphicsPipeline");
  luaPushcfunction2(L, lGraphicsResetGraphicsPipeline);
  lua_setfield(L, -2, "resetGraphicsPipeline");
  luaPushcfunction2(L, lGraphicsSetTexture);
  lua_setfield(L, -2, "setTexture");
  luaPushcfunction2(L, lGraphicsResetTexture);
  lua_setfield(L, -2, "resetTexture");
  luaPushcfunction2(L, lGraphicsResetAllTexture);
  lua_setfield(L, -2, "resetAllTexture");
  luaPushcfunction2(L, lGraphicsSetUniformBuffer);
  lua_setfield(L, -2, "setUniformBuffer");
  luaPushcfunction2(L, lGraphicsBeginRenderTarget);
  lua_setfield(L, -2, "beginRenderTarget");
  luaPushcfunction2(L, lGraphicsEndRenderTarget);
  lua_setfield(L, -2, "endRenderTarget");
  luaPushcfunction2(L, lGraphicsReadbackTexture);
  lua_setfield(L, -2, "readbackTexture");
  lua_pop(L, 1);
}

} // namespace sinen
