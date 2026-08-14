#include "luaapi.hpp"

#include <core/allocator/global_allocator.hpp>
#include <core/profiler.hpp>
#include <gpu/gpu.hpp>
#include <gpu/shader/builtin_shader.hpp>
#include <graphics/builtin_pipeline.hpp>
#include <graphics/graphics.hpp>
#include <graphics/gui/gui.hpp>
#include <graphics/render_pass.hpp>
#include <graphics/texture/render_texture.hpp>
#include <math/transform/transform.hpp>
#include <platform/io/asset_reader.hpp>
#include <platform/window/window.hpp>

#include <cctype>

namespace sinen {
static String normalizedBackendName(const char *name) {
  String normalized = name ? name : "";
  for (auto &ch : normalized) {
    ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  }
  return normalized;
}

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
static int lGraphicsBegin2DPass(lua_State *L) {
  Ptr<Render2DPass> pass;
  const int count = lua_gettop(L);
  if (count == 0 || lua_isnil(L, 1)) {
    pass = Graphics::begin2DPass();
  } else if (luaLTestudata2(L, 1, Camera2D::metaTableName())) {
    auto &camera = udValue<Camera2D>(L, 1);
    if (count >= 2 && !lua_isnil(L, 2)) {
      pass = Graphics::begin2DPass(camera, *udPtr<RenderTexture>(L, 2));
    } else {
      pass = Graphics::begin2DPass(camera);
    }
  } else if (luaLTestudata2(L, 1, RenderTexture::metaTableName())) {
    pass = Graphics::begin2DPass(*udPtr<RenderTexture>(L, 1));
  } else {
    luaL_error(L, "begin2DPass expects Camera2D and/or RenderTexture");
    return 0;
  }
  if (!pass) {
    luaL_error(L, "failed to begin a 2D render pass");
    return 0;
  }
  udPushPtr<Render2DPass>(L, std::move(pass));
  return 1;
}
static int lGraphicsBegin3DPass(lua_State *L) {
  auto &camera = udValue<Camera3D>(L, 1);
  Ptr<Render3DPass> pass;
  if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
    pass = Graphics::begin3DPass(camera, *udPtr<RenderTexture>(L, 2));
  } else {
    pass = Graphics::begin3DPass(camera);
  }
  if (!pass) {
    luaL_error(L, "failed to begin a 3D render pass");
    return 0;
  }
  udPushPtr<Render3DPass>(L, std::move(pass));
  return 1;
}
static int lGraphicsEndPass(lua_State *L) {
  if (luaLTestudata2(L, 1, Render2DPass::metaTableName())) {
    Graphics::endPass(udPtr<Render2DPass>(L, 1));
    return 0;
  }
  if (luaLTestudata2(L, 1, Render3DPass::metaTableName())) {
    Graphics::endPass(udPtr<Render3DPass>(L, 1));
    return 0;
  }
  luaL_error(L, "endPass expects Render2DPass or Render3DPass");
  return 0;
}

template <class T> static int lRenderPassIsFinished(lua_State *L) {
  lua_pushboolean(L, udPtr<T>(L, 1)->isFinished());
  return 1;
}
template <class T> static int lRenderPassSetGraphicsPipeline(lua_State *L) {
  auto &pipeline = udPtr<GraphicsPipeline>(L, 2);
  udPtr<T>(L, 1)->setGraphicsPipeline(*pipeline);
  return 0;
}
template <class T> static int lRenderPassResetGraphicsPipeline(lua_State *L) {
  udPtr<T>(L, 1)->resetGraphicsPipeline();
  return 0;
}
template <class T> static int lRenderPassSetUniformBuffer(lua_State *L) {
  auto &pass = udPtr<T>(L, 1);
  auto &buffer = udValue<Buffer>(L, 3);
  if (lua_type(L, 2) == LUA_TSTRING) {
    pass->setUniformBuffer(StringView(luaL_checkstring(L, 2)), buffer);
  } else {
    pass->setUniformBuffer(static_cast<UInt32>(luaL_checkinteger(L, 2)),
                           buffer);
  }
  return 0;
}
template <class T> static int lRenderPassSetTexture(lua_State *L) {
  auto &pass = udPtr<T>(L, 1);
  auto &texture = udPtr<Texture>(L, 3);
  if (lua_type(L, 2) == LUA_TSTRING) {
    pass->setTexture(StringView(luaL_checkstring(L, 2)), texture);
  } else {
    pass->setTexture(static_cast<UInt32>(luaL_checkinteger(L, 2)), texture);
  }
  return 0;
}
template <class T> static int lRenderPassResetTexture(lua_State *L) {
  auto &pass = udPtr<T>(L, 1);
  if (lua_type(L, 2) == LUA_TSTRING) {
    pass->resetTexture(StringView(luaL_checkstring(L, 2)));
  } else {
    pass->resetTexture(static_cast<UInt32>(luaL_checkinteger(L, 2)));
  }
  return 0;
}
template <class T> static int lRenderPassResetAllTexture(lua_State *L) {
  udPtr<T>(L, 1)->resetAllTexture();
  return 0;
}
static int lRender2DPassWindowToCurrent(lua_State *L) {
  auto position = udValue<Vec2>(L, 2);
  udNewOwned<Vec2>(L, udPtr<Render2DPass>(L, 1)->windowToCurrent(position));
  return 1;
}
static int lRender2DPassDrawRect(lua_State *L) {
  auto &pass = udPtr<Render2DPass>(L, 1);
  auto &rect = udValue<Rect>(L, 2);
  auto &color = udValue<Color>(L, 3);
  const float angle =
      lua_gettop(L) >= 4 ? static_cast<float>(luaL_checknumber(L, 4)) : 0.0f;
  pass->drawRect(rect, color, angle);
  return 0;
}
static int lRender2DPassDrawImage(lua_State *L) {
  auto &pass = udPtr<Render2DPass>(L, 1);
  auto &texture = udPtr<Texture>(L, 2);
  auto &rect = udValue<Rect>(L, 3);
  const float angle =
      lua_gettop(L) >= 4 ? static_cast<float>(luaL_checknumber(L, 4)) : 0.0f;
  pass->drawImage(texture, rect, angle);
  return 0;
}
static int lRender2DPassDrawText(lua_State *L) {
  auto &pass = udPtr<Render2DPass>(L, 1);
  const char *text = luaL_checkstring(L, 2);
  auto &style = udValue<TextStyle>(L, 3);
  auto &transform = udValue<TextTransform>(L, 4);
  pass->drawText(StringView(text), style, transform);
  return 0;
}
static int lRender3DPassDrawCubemap(lua_State *L) {
  udPtr<Render3DPass>(L, 1)->drawCubemap(udPtr<Texture>(L, 2));
  return 0;
}
static int lRender3DPassDrawModel(lua_State *L) {
  auto &model = udPtr<Model>(L, 2);
  auto &transform = udValue<Transform>(L, 3);
  udPtr<Render3DPass>(L, 1)->drawModel(*model, transform);
  return 0;
}
static int lRender3DPassDrawModelInstanced(lua_State *L) {
  auto &model = udPtr<Model>(L, 2);
  luaL_checktype(L, 3, LUA_TTABLE);
  Array<Transform> transforms;
  const size_t count = lua_objlen(L, 3);
  transforms.reserve(count);
  for (size_t i = 1; i <= count; ++i) {
    lua_rawgeti(L, 3, static_cast<lua_Integer>(i));
    transforms.push_back(udValue<Transform>(L, -1));
    lua_pop(L, 1);
  }
  udPtr<Render3DPass>(L, 1)->drawModelInstanced(*model, transforms);
  return 0;
}

template <class T> static void registerRenderPassCommon(lua_State *L) {
  Binding::registerFunction(L, "isFinished", lRenderPassIsFinished<T>);
  Binding::registerFunction(L, "setGraphicsPipeline",
                            lRenderPassSetGraphicsPipeline<T>);
  Binding::registerFunction(L, "resetGraphicsPipeline",
                            lRenderPassResetGraphicsPipeline<T>);
  Binding::registerFunction(L, "setUniformBuffer",
                            lRenderPassSetUniformBuffer<T>);
  Binding::registerFunction(L, "setTexture", lRenderPassSetTexture<T>);
  Binding::registerFunction(L, "resetTexture", lRenderPassResetTexture<T>);
  Binding::registerFunction(L, "resetAllTexture",
                            lRenderPassResetAllTexture<T>);
}

static void registerRenderPasses(lua_State *L) {
  luaL_newmetatable(L, Render2DPass::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  registerRenderPassCommon<Render2DPass>(L);
  Binding::registerFunction(L, "windowToCurrent", lRender2DPassWindowToCurrent);
  Binding::registerFunction(L, "drawRect", lRender2DPassDrawRect);
  Binding::registerFunction(L, "drawImage", lRender2DPassDrawImage);
  Binding::registerFunction(L, "drawText", lRender2DPassDrawText);
  lua_pop(L, 1);

  luaL_newmetatable(L, Render3DPass::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  registerRenderPassCommon<Render3DPass>(L);
  Binding::registerFunction(L, "drawCubemap", lRender3DPassDrawCubemap);
  Binding::registerFunction(L, "drawModel", lRender3DPassDrawModel);
  Binding::registerFunction(L, "drawModelInstanced",
                            lRender3DPassDrawModelInstanced);
  lua_pop(L, 1);
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
static int lGraphicsSetMSAASampleCount(lua_State *L) {
  UInt32 sampleCount = static_cast<UInt32>(luaL_checkinteger(L, 1));
  if (sampleCount != 1 && sampleCount != 2 && sampleCount != 4 &&
      sampleCount != 8) {
    luaL_error(L, "MSAA sample count must be 1, 2, 4, or 8");
    return 0;
  }
  Graphics::setMSAASampleCount(sampleCount);
  return 0;
}
static int lGraphicsGetMSAASampleCount(lua_State *L) {
  lua_pushinteger(L, Graphics::getMSAASampleCount());
  return 1;
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
  auto &t = udPtr<Texture>(L, 2);
  if (lua_type(L, 1) == LUA_TSTRING) {
    const char *name = luaL_checkstring(L, 1);
    Graphics::setTexture(StringView(name), t);
  } else {
    UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
    Graphics::setTexture(slot, t);
  }
  return 0;
}
static int lGraphicsResetTexture(lua_State *L) {
  if (lua_type(L, 1) == LUA_TSTRING) {
    const char *name = luaL_checkstring(L, 1);
    Graphics::resetTexture(StringView(name));
  } else {
    UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
    Graphics::resetTexture(slot);
  }
  return 0;
}
static int lGraphicsResetAllTexture(lua_State *L) {
  (void)L;
  Graphics::resetAllTexture();
  return 0;
}
static int lGraphicsSetUniformBuffer(lua_State *L) {
  auto &b = udValue<Buffer>(L, 2);
  if (lua_type(L, 1) == LUA_TSTRING) {
    const char *name = luaL_checkstring(L, 1);
    Graphics::setUniformBuffer(StringView(name), b);
  } else {
    UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
    Graphics::setUniformBuffer(slot, b);
  }
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
static int lGraphicsGetBackendName(lua_State *L) {
  const auto name = Graphics::getBackendName();
  lua_pushstring(L, name.c_str());
  return 1;
}
void registerGraphics(lua_State *L) {
  registerRenderPasses(L);
  pushSnNamed(L, "Graphics");
  Binding::registerFunction(L, "drawRect", lGraphicsDrawRect);
  Binding::registerFunction(L, "drawText", lGraphicsDrawText);
  Binding::registerFunction(L, "drawImage", lGraphicsDrawImage);
  Binding::registerFunction(L, "drawCubemap", lGraphicsDrawCubemap);
  Binding::registerFunction(L, "drawModel", lGraphicsDrawModel);
  Binding::registerFunction(L, "drawModelInstanced",
                            lGraphicsDrawModelInstanced);
  Binding::registerFunction(L, "begin2D", lGraphicsBegin2D);
  Binding::registerFunction(L, "begin3D", lGraphicsBegin3D);
  Binding::registerFunction(L, "finish", lGraphicsFinish);
  Binding::registerFunction(L, "begin2DPass", lGraphicsBegin2DPass);
  Binding::registerFunction(L, "begin3DPass", lGraphicsBegin3DPass);
  Binding::registerFunction(L, "endPass", lGraphicsEndPass);
  Binding::registerFunction(L, "getClearColor", lGraphicsGetClearColor);
  Binding::registerFunction(L, "setClearColor", lGraphicsSetClearColor);
  Binding::registerFunction(L, "setMSAASampleCount",
                            lGraphicsSetMSAASampleCount);
  Binding::registerFunction(L, "getMSAASampleCount",
                            lGraphicsGetMSAASampleCount);
  Binding::registerFunction(L, "setGraphicsPipeline",
                            lGraphicsSetGraphicsPipeline);
  Binding::registerFunction(L, "resetGraphicsPipeline",
                            lGraphicsResetGraphicsPipeline);
  Binding::registerFunction(L, "setTexture", lGraphicsSetTexture);
  Binding::registerFunction(L, "resetTexture", lGraphicsResetTexture);
  Binding::registerFunction(L, "resetAllTexture", lGraphicsResetAllTexture);
  Binding::registerFunction(L, "setUniformBuffer", lGraphicsSetUniformBuffer);
  Binding::registerFunction(L, "beginRenderTarget", lGraphicsBeginRenderTarget);
  Binding::registerFunction(L, "endRenderTarget", lGraphicsEndRenderTarget);
  Binding::registerFunction(L, "readbackTexture", lGraphicsReadbackTexture);
  Binding::registerFunction(L, "getBackendName", lGraphicsGetBackendName);
  lua_pop(L, 1);
}

} // namespace sinen
