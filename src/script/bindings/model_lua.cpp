#include "luaapi.hpp"
#include <core/allocator/global_allocator.hpp>
#include <core/buffer/buffer.hpp>
#include <core/core.hpp>
#include <core/data/ptr.hpp>
#include <core/profiler.hpp>
#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>
#include <graphics/graphics.hpp>
#include <graphics/model/model.hpp>
#include <math/geometry/skinned_vertex.hpp>
#include <math/geometry/vertex.hpp>
#include <math/mat4.hpp>
#include <math/math.hpp>
#include <math/matrix.hpp>
#include <math/quaternion.hpp>
#include <math/vector.hpp>
#include <platform/io/asset_reader.hpp>
#include <platform/io/external_file.hpp>

namespace sinen {
static int lModelNew(lua_State *L) {
  if (lua_gettop(L) < 1 || lua_gettop(L) > 2) {
    return luaLError2(
        L, "sn.Model.new expects a source and optional load options");
  }
  Model::LoadOptions options;
  if (!lua_isnoneornil(L, 2)) {
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_getfield(L, 2, "maxTextureSize");
    if (!lua_isnil(L, -1))
      options.maxTextureSize =
          static_cast<UInt32>(std::clamp(luaL_checkinteger(L, -1), 0, 16384));
    lua_pop(L, 1);
    lua_getfield(L, 2, "packedSpecular");
    options.packedSpecular = lua_toboolean(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, 2, "flipNormalY");
    options.flipNormalY = lua_toboolean(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, 2, "staticScene");
    options.staticScene = lua_toboolean(L, -1);
    lua_pop(L, 1);
  }
  auto model = makePtr<Model>();
  if (lua_isstring(L, 1)) {
    const char *path = luaL_checkstring(L, 1);
    if (!AssetReader::exists(path)) {
      model.reset();
      return luaLError2(L, "sn.Model.new asset not found: %s", path);
    }
    model->load(StringView(path), options);
  } else if (auto *file = udValueOrNull<ExternalFile>(L, 1)) {
    auto buffer = file->read();
    if (!buffer || buffer->size() == 0) {
      model.reset();
      return luaLError2(L, "sn.Model.new external file is empty or unreadable");
    }
    String hint = file->extension();
    if (!hint.empty() && hint.front() == '.') {
      hint.erase(hint.begin());
    }
    model->load(*buffer, hint, options);
  } else {
    auto &buffer = udValue<Buffer>(L, 1);
    if (buffer.size() == 0) {
      model.reset();
      return luaLError2(L, "sn.Model.new source buffer is empty");
    }
    model->load(buffer, "", options);
  }
  udPushPtr<Model>(L, std::move(model));
  return 1;
}
static int lModelIsReady(lua_State *L) {
  lua_pushboolean(L, udPtr<Model>(L, 1)->isReady());
  return 1;
}
static auto lModelGetPartCount(const Ptr<Model> &model) {
  return model->getParts().empty() ? (model->isReady() ? 1 : 0)
                                   : model->getParts().size();
}
static int lModelGetMaterialCount(lua_State *L) {
  lua_pushinteger(L, udPtr<Model>(L, 1)->getMaterialCount());
  return 1;
}
static int lModelGetTriangleCount(lua_State *L) {
  lua_pushinteger(L, udPtr<Model>(L, 1)->getTriangleCount());
  return 1;
}
static auto lModelGetAabb(Ptr<Model> &m) { return m->getAABB(); }
static auto lModelNewSprite() {
  auto model = makePtr<Model>();
  model->loadSprite();
  return std::move(model);
}
static auto lModelNewBox() {
  auto model = makePtr<Model>();
  model->loadBox();
  return std::move(model);
}
static auto lModelGetBoneUniformBuffer(Ptr<Model> &m) {
  return m->getBoneUniformBuffer();
}
static auto lModelPlay(Ptr<Model> &m, float start) {
  m->play(start);
  return;
}
static auto lModelUpdate(Ptr<Model> &m, float dt) {
  m->update(dt);
  return;
}
static int lModelHasTexture(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  auto k = static_cast<TextureKey>(luaL_checkinteger(L, 2));
  lua_pushboolean(L, m->hasTexture(k));
  return 1;
}
static int lModelGetTexture(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  auto k = static_cast<TextureKey>(luaL_checkinteger(L, 2));
  udPushPtr<Texture>(L, m->getTexture(k));
  return 1;
}
static int lModelSetTexture(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  auto k = static_cast<TextureKey>(luaL_checkinteger(L, 2));
  auto &t = udPtr<Texture>(L, 3);
  m->setTexture(k, t);
  return 0;
}
void registerModel(lua_State *L) {
  pushClassMetatable<Ptr<Model>>(L, Model::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luau::table(L, -1).function("getAABB", lModelGetAabb);
  luau::table(L, -1).raw_function("isReady", lModelIsReady);
  luau::table(L, -1).function("getPartCount", lModelGetPartCount);
  luau::table(L, -1).raw_function("getMaterialCount", lModelGetMaterialCount);
  luau::table(L, -1).raw_function("getTriangleCount", lModelGetTriangleCount);
  luau::table(L, -1).function("getBoneUniformBuffer",
                              lModelGetBoneUniformBuffer);
  luau::table(L, -1).function("play", lModelPlay);
  luau::table(L, -1).function("update", lModelUpdate);
  luau::table(L, -1).raw_function("hasTexture", lModelHasTexture);
  luau::table(L, -1).raw_function("getTexture", lModelGetTexture);
  luau::table(L, -1).raw_function("setTexture", lModelSetTexture);
  lua_pop(L, 1);

  pushSnNamed(L, "Model");
  luau::table(L, -1).raw_function("new", lModelNew);
  luau::table(L, -1).function("newSprite", lModelNewSprite);
  luau::table(L, -1).function("newBox", lModelNewBox);
  lua_pop(L, 1);
}

void registerTextureKey(lua_State *L) {
  pushSnNamed(L, "TextureKey");
  luau::table(L, -1).value("BaseColor",
                           static_cast<lua_Integer>(TextureKey::BaseColor));
  luau::table(L, -1).value("Normal",
                           static_cast<lua_Integer>(TextureKey::Normal));
  luau::table(L, -1).value(
      "DiffuseRoughness",
      static_cast<lua_Integer>(TextureKey::DiffuseRoughness));
  luau::table(L, -1).value("Metalness",
                           static_cast<lua_Integer>(TextureKey::Metalness));
  luau::table(L, -1).value("Emissive",
                           static_cast<lua_Integer>(TextureKey::Emissive));
  luau::table(L, -1).value("LightMap",
                           static_cast<lua_Integer>(TextureKey::LightMap));
  lua_pop(L, 1);
}
} // namespace sinen
