#include "luaapi.hpp"

#include <core/allocator/global_allocator.hpp>
#include <core/profiler.hpp>
#include <gpu/gpu.hpp>
#include <gpu/renderdoc.hpp>
#include <gpu/shader/builtin_shader.hpp>
#include <graphics/builtin_pipeline.hpp>
#include <graphics/graphics.hpp>
#include <graphics/gui/gui.hpp>
#include <graphics/material.hpp>
#include <graphics/render_pass.hpp>
#include <graphics/texture/render_texture.hpp>
#include <math/transform/transform.hpp>
#include <platform/io/asset_reader.hpp>
#include <platform/window/window.hpp>

#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>

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
static auto lGraphicsDrawText(const char *text, TextStyle &style,
                              TextTransform &transform) {
  Graphics::drawText(StringView(text), style, transform);
  return;
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
static auto lGraphicsDrawCubemap(Ptr<Texture> &tex) {
  Graphics::drawCubemap(tex);
  return;
}
static auto lGraphicsDrawModel(Ptr<Model> &m, Transform &t) {
  Graphics::drawModel(*m, t);
  return;
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
static auto lGraphicsBegin3D(Camera3D &cam) {
  Graphics::begin3D(cam);
  return;
}
static auto lGraphicsFinish() {
  Graphics::finish();
  return;
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

static auto lMaterialNew(Ptr<GraphicsPipeline> &pipeline) {
  return makePtr<Material>(GlobalAllocator::get(), pipeline);
}
static int lMaterialSetUniformBuffer(lua_State *L) {
  auto &material = udPtr<Material>(L, 1);
  auto &buffer = udValue<Buffer>(L, 3);
  if (lua_type(L, 2) == LUA_TSTRING) {
    material->setUniformBuffer(StringView(luaL_checkstring(L, 2)), buffer);
  } else {
    material->setUniformBuffer(static_cast<UInt32>(luaL_checkinteger(L, 2)),
                               buffer);
  }
  return 0;
}
static int lMaterialResetUniformBuffer(lua_State *L) {
  auto &material = udPtr<Material>(L, 1);
  if (lua_type(L, 2) == LUA_TSTRING) {
    material->resetUniformBuffer(StringView(luaL_checkstring(L, 2)));
  } else {
    material->resetUniformBuffer(static_cast<UInt32>(luaL_checkinteger(L, 2)));
  }
  return 0;
}
static int lMaterialResetAllUniformBuffers(lua_State *L) {
  udPtr<Material>(L, 1)->resetAllUniformBuffers();
  return 0;
}
static int lMaterialSetTexture(lua_State *L) {
  auto &material = udPtr<Material>(L, 1);
  auto &texture = udPtr<Texture>(L, 3);
  if (lua_type(L, 2) == LUA_TSTRING) {
    material->setTexture(StringView(luaL_checkstring(L, 2)), texture);
  } else {
    material->setTexture(static_cast<UInt32>(luaL_checkinteger(L, 2)), texture);
  }
  return 0;
}
static int lMaterialResetTexture(lua_State *L) {
  auto &material = udPtr<Material>(L, 1);
  if (lua_type(L, 2) == LUA_TSTRING) {
    material->resetTexture(StringView(luaL_checkstring(L, 2)));
  } else {
    material->resetTexture(static_cast<UInt32>(luaL_checkinteger(L, 2)));
  }
  return 0;
}
static int lMaterialResetAllTexture(lua_State *L) {
  udPtr<Material>(L, 1)->resetAllTexture();
  return 0;
}

static void registerMaterial(luau::table &sn) {
  auto material = engineClass<Ptr<Material>>(sn, Material::metaTableName());

  material.raw_method<lMaterialSetUniformBuffer>("setUniformBuffer");
  material.raw_method<lMaterialResetUniformBuffer>("resetUniformBuffer");
  material.raw_method<lMaterialResetAllUniformBuffers>(
      "resetAllUniformBuffers");
  material.raw_method<lMaterialSetTexture>("setTexture");
  material.raw_method<lMaterialResetTexture>("resetTexture");
  material.raw_method<lMaterialResetAllTexture>("resetAllTexture");

  material.static_function<lMaterialNew>("new");
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
  if (luaLTestudata2(L, 4, Material::metaTableName())) {
    const float angle =
        lua_gettop(L) >= 5 ? static_cast<float>(luaL_checknumber(L, 5)) : 0.0f;
    pass->drawRect(rect, color, *udPtr<Material>(L, 4), angle);
  } else {
    const float angle =
        lua_gettop(L) >= 4 ? static_cast<float>(luaL_checknumber(L, 4)) : 0.0f;
    pass->drawRect(rect, color, angle);
  }
  return 0;
}
static int lRender2DPassDrawImage(lua_State *L) {
  auto &pass = udPtr<Render2DPass>(L, 1);
  auto &texture = udPtr<Texture>(L, 2);
  auto &rect = udValue<Rect>(L, 3);
  if (luaLTestudata2(L, 4, Material::metaTableName())) {
    const float angle =
        lua_gettop(L) >= 5 ? static_cast<float>(luaL_checknumber(L, 5)) : 0.0f;
    pass->drawImage(texture, rect, *udPtr<Material>(L, 4), angle);
  } else {
    const float angle =
        lua_gettop(L) >= 4 ? static_cast<float>(luaL_checknumber(L, 4)) : 0.0f;
    pass->drawImage(texture, rect, angle);
  }
  return 0;
}
static int lRender2DPassDrawText(lua_State *L) {
  auto &pass = udPtr<Render2DPass>(L, 1);
  const char *text = luaL_checkstring(L, 2);
  auto &style = udValue<TextStyle>(L, 3);
  auto &transform = udValue<TextTransform>(L, 4);
  if (luaLTestudata2(L, 5, Material::metaTableName())) {
    pass->drawText(StringView(text), style, transform, *udPtr<Material>(L, 5));
  } else {
    pass->drawText(StringView(text), style, transform);
  }
  return 0;
}
static int lRender3DPassDrawCubemap(lua_State *L) {
  auto &pass = udPtr<Render3DPass>(L, 1);
  auto &texture = udPtr<Texture>(L, 2);
  if (luaLTestudata2(L, 3, Material::metaTableName())) {
    pass->drawCubemap(texture, *udPtr<Material>(L, 3));
  } else {
    pass->drawCubemap(texture);
  }
  return 0;
}
static int lRender3DPassDrawModel(lua_State *L) {
  auto &model = udPtr<Model>(L, 2);
  auto &transform = udValue<Transform>(L, 3);
  auto &pass = udPtr<Render3DPass>(L, 1);
  if (luaLTestudata2(L, 4, Material::metaTableName())) {
    pass->drawModel(*model, transform, *udPtr<Material>(L, 4));
  } else {
    pass->drawModel(*model, transform);
  }
  return 0;
}
static int lRender3DPassDrawMesh(lua_State *L) {
  auto &pass = udPtr<Render3DPass>(L, 1);
  size_t vertexBytes = 0, indexBytes = 0;
  const auto *vertices =
      static_cast<const char *>(luaL_checkbuffer(L, 2, &vertexBytes));
  const auto *indices =
      static_cast<const char *>(luaL_checkbuffer(L, 3, &indexBytes));
  auto &transform = udValue<Transform>(L, 4);
  // Wire layout: 12 float32 values (position, normal, UV, RGBA), uint32
  // indices.
  static_assert(sizeof(Vertex) == 12 * sizeof(float));
  static_assert(offsetof(Vertex, normal) == 3 * sizeof(float));
  static_assert(offsetof(Vertex, uv) == 6 * sizeof(float));
  static_assert(offsetof(Vertex, color) == 8 * sizeof(float));
  if (vertexBytes == 0 || vertexBytes % sizeof(Vertex) != 0 ||
      indexBytes == 0 || indexBytes % (3 * sizeof(UInt32)) != 0 ||
      vertexBytes > std::numeric_limits<UInt32>::max() ||
      indexBytes > std::numeric_limits<UInt32>::max()) {
    return luaLError2(
        L, "drawMesh expects float32 vertex records and uint32 triangles");
  }
  Mesh mesh;
  auto data = mesh.data();
  data->vertices.resize(vertexBytes / sizeof(Vertex));
  data->indices.resize(indexBytes / sizeof(UInt32));
  std::memcpy(data->vertices.data(), vertices, vertexBytes);
  std::memcpy(data->indices.data(), indices, indexBytes);
  for (const auto index : data->indices) {
    if (index >= data->vertices.size()) {
      return luaLError2(L, "drawMesh index is outside the vertex buffer");
    }
  }
  for (size_t offset = 0; offset < vertexBytes; offset += sizeof(float)) {
    float value;
    std::memcpy(&value, vertices + offset, sizeof(value));
    if (!std::isfinite(value)) {
      return luaLError2(L, "drawMesh vertices must be finite");
    }
  }
  if (!lua_isnoneornil(L, 5)) {
    pass->drawMesh(mesh, transform, *udPtr<Material>(L, 5));
  } else {
    pass->drawMesh(mesh, transform);
  }
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
  auto &pass = udPtr<Render3DPass>(L, 1);
  if (luaLTestudata2(L, 4, Material::metaTableName())) {
    pass->drawModelInstanced(*model, transforms, *udPtr<Material>(L, 4));
  } else {
    pass->drawModelInstanced(*model, transforms);
  }
  return 0;
}

template <class T>
static void registerRenderPassCommon(luau::class_builder<Ptr<T>> &binding) {
  binding.template raw_method<lRenderPassIsFinished<T>>("isFinished");
  binding.template raw_method<lRenderPassSetGraphicsPipeline<T>>(
      "setGraphicsPipeline");
  binding.template raw_method<lRenderPassResetGraphicsPipeline<T>>(
      "resetGraphicsPipeline");
  binding.template raw_method<lRenderPassSetUniformBuffer<T>>(
      "setUniformBuffer");
  binding.template raw_method<lRenderPassSetTexture<T>>("setTexture");
  binding.template raw_method<lRenderPassResetTexture<T>>("resetTexture");
  binding.template raw_method<lRenderPassResetAllTexture<T>>("resetAllTexture");
}

static void registerRenderPasses(luau::table &sn) {
  auto render2DPass =
      engineClass<Ptr<Render2DPass>>(sn, Render2DPass::metaTableName());

  registerRenderPassCommon<Render2DPass>(render2DPass);
  render2DPass.raw_method<lRender2DPassWindowToCurrent>("windowToCurrent");
  render2DPass.raw_method<lRender2DPassDrawRect>("drawRect");
  render2DPass.raw_method<lRender2DPassDrawImage>("drawImage");
  render2DPass.raw_method<lRender2DPassDrawText>("drawText");

  auto render3DPass =
      engineClass<Ptr<Render3DPass>>(sn, Render3DPass::metaTableName());

  registerRenderPassCommon<Render3DPass>(render3DPass);
  render3DPass.raw_method<lRender3DPassDrawCubemap>("drawCubemap");
  render3DPass.raw_method<lRender3DPassDrawModel>("drawModel");
  render3DPass.raw_method<lRender3DPassDrawMesh>("drawMesh");
  render3DPass.raw_method<lRender3DPassDrawModelInstanced>(
      "drawModelInstanced");
}
static auto lGraphicsGetClearColor() { return Graphics::getClearColor(); }
static auto lGraphicsSetClearColor(Color &c) {
  Graphics::setClearColor(c);
  return;
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
static auto lGraphicsGetMSAASampleCount() {
  return Graphics::getMSAASampleCount();
}
static auto lGraphicsSetGraphicsPipeline(Ptr<GraphicsPipeline> &p) {
  Graphics::setGraphicsPipeline(*p);
  return;
}
static auto lGraphicsResetGraphicsPipeline() {
  Graphics::resetGraphicsPipeline();
  return;
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
static auto lGraphicsResetAllTexture() {
  Graphics::resetAllTexture();
  return;
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
static auto lGraphicsBeginRenderTarget(Ptr<RenderTexture> &rt) {
  Graphics::beginRenderTarget(*rt);
  return;
}
static auto lGraphicsEndRenderTarget() {
  Graphics::endRenderTarget();
  return;
}
static auto lGraphicsReadbackTexture(Ptr<RenderTexture> &rt,
                                     Ptr<Texture> &out) {
  return Graphics::readbackTexture(*rt, out);
}
static int lGraphicsGetBackendName(lua_State *L) {
  const auto name = Graphics::getBackendName();
  lua_pushstring(L, name.c_str());
  return 1;
}
static auto lRenderDocIsAvailable() { return RenderDoc::isAvailable(); }
static int lRenderDocTriggerCapture(lua_State *L) {
  const int frames = luaL_optinteger(L, 1, 1);
  if (frames <= 0) {
    luaL_error(L, "capture frame count must be positive");
    return 0;
  }
  lua_pushboolean(L, RenderDoc::triggerCapture(static_cast<UInt32>(frames)));
  return 1;
}
static auto lRenderDocIsFrameCapturing() {
  return RenderDoc::isFrameCapturing();
}
static int lRenderDocSetCaptureFilePathTemplate(lua_State *L) {
  lua_pushboolean(
      L, RenderDoc::setCaptureFilePathTemplate(luaL_checkstring(L, 1)));
  return 1;
}
static auto lRenderDocGetNumCaptures() { return RenderDoc::getNumCaptures(); }
static int lRenderDocGetCaptureFilePath(lua_State *L) {
  const int index = luaL_checkinteger(L, 1);
  if (index < 0) {
    luaL_error(L, "capture index must be non-negative");
    return 0;
  }
  const auto path = RenderDoc::getCaptureFilePath(static_cast<UInt32>(index));
  lua_pushlstring(L, path.data(), path.size());
  return 1;
}
void registerGraphics(luau::table &sn) {
  auto renderDoc = sn.get_or_create_table("RenderDoc");
  renderDoc.function<lRenderDocIsAvailable>("isAvailable");
  renderDoc.raw_function<lRenderDocTriggerCapture>("triggerCapture");
  renderDoc.function<lRenderDocIsFrameCapturing>("isFrameCapturing");
  renderDoc.raw_function<lRenderDocSetCaptureFilePathTemplate>(
      "setCaptureFilePathTemplate");
  renderDoc.function<lRenderDocGetNumCaptures>("getNumCaptures");
  renderDoc.raw_function<lRenderDocGetCaptureFilePath>("getCaptureFilePath");

  registerMaterial(sn);
  registerRenderPasses(sn);
  auto graphics = sn.get_or_create_table("Graphics");
  graphics.raw_function<lGraphicsDrawRect>("drawRect");
  graphics.function<lGraphicsDrawText>("drawText");
  graphics.raw_function<lGraphicsDrawImage>("drawImage");
  graphics.function<lGraphicsDrawCubemap>("drawCubemap");
  graphics.function<lGraphicsDrawModel>("drawModel");
  graphics.raw_function<lGraphicsDrawModelInstanced>("drawModelInstanced");
  graphics.raw_function<lGraphicsBegin2D>("begin2D");
  graphics.function<lGraphicsBegin3D>("begin3D");
  graphics.function<lGraphicsFinish>("finish");
  graphics.raw_function<lGraphicsBegin2DPass>("begin2DPass");
  graphics.raw_function<lGraphicsBegin3DPass>("begin3DPass");
  graphics.raw_function<lGraphicsEndPass>("endPass");
  graphics.function<lGraphicsGetClearColor>("getClearColor");
  graphics.function<lGraphicsSetClearColor>("setClearColor");
  graphics.raw_function<lGraphicsSetMSAASampleCount>("setMSAASampleCount");
  graphics.function<lGraphicsGetMSAASampleCount>("getMSAASampleCount");
  graphics.function<lGraphicsSetGraphicsPipeline>("setGraphicsPipeline");
  graphics.function<lGraphicsResetGraphicsPipeline>("resetGraphicsPipeline");
  graphics.raw_function<lGraphicsSetTexture>("setTexture");
  graphics.raw_function<lGraphicsResetTexture>("resetTexture");
  graphics.function<lGraphicsResetAllTexture>("resetAllTexture");
  graphics.raw_function<lGraphicsSetUniformBuffer>("setUniformBuffer");
  graphics.function<lGraphicsBeginRenderTarget>("beginRenderTarget");
  graphics.function<lGraphicsEndRenderTarget>("endRenderTarget");
  graphics.function<lGraphicsReadbackTexture>("readbackTexture");
  graphics.raw_function<lGraphicsGetBackendName>("getBackendName");
}
} // namespace sinen
