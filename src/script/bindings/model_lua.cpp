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
  if (lua_gettop(L) != 1) {
    return luaLError2(L, "sn.Model.new expects exactly one source");
  }
  auto model = makePtr<Model>();
  if (lua_isstring(L, 1)) {
    const char *path = luaL_checkstring(L, 1);
    if (!AssetReader::exists(path)) {
      model.reset();
      return luaLError2(L, "sn.Model.new asset not found: %s", path);
    }
    model->load(StringView(path));
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
    model->load(*buffer, hint);
  } else {
    auto &buffer = udValue<Buffer>(L, 1);
    if (buffer.size() == 0) {
      model.reset();
      return luaLError2(L, "sn.Model.new source buffer is empty");
    }
    model->load(buffer);
  }
  udPushPtr<Model>(L, std::move(model));
  return 1;
}
static int lModelGetAabb(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  udNewOwned<AABB>(L, m->getAABB());
  return 1;
}
static int lModelNewSprite(lua_State *L) {
  auto model = makePtr<Model>();
  model->loadSprite();
  udPushPtr<Model>(L, std::move(model));
  return 1;
}
static int lModelNewBox(lua_State *L) {
  auto model = makePtr<Model>();
  model->loadBox();
  udPushPtr<Model>(L, std::move(model));
  return 1;
}
static int lModelGetBoneUniformBuffer(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  udNewOwned<Buffer>(L, m->getBoneUniformBuffer());
  return 1;
}
static int lModelPlay(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  float start = static_cast<float>(luaL_checknumber(L, 2));
  m->play(start);
  return 0;
}
static int lModelUpdate(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  float dt = static_cast<float>(luaL_checknumber(L, 2));
  m->update(dt);
  return 0;
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
  luaL_newmetatable(L, Model::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "getAABB", lModelGetAabb);
  Binding::registerFunction(L, "getBoneUniformBuffer",
                            lModelGetBoneUniformBuffer);
  Binding::registerFunction(L, "play", lModelPlay);
  Binding::registerFunction(L, "update", lModelUpdate);
  Binding::registerFunction(L, "hasTexture", lModelHasTexture);
  Binding::registerFunction(L, "getTexture", lModelGetTexture);
  Binding::registerFunction(L, "setTexture", lModelSetTexture);
  lua_pop(L, 1);

  pushSnNamed(L, "Model");
  Binding::registerFunction(L, "new", lModelNew);
  Binding::registerFunction(L, "newSprite", lModelNewSprite);
  Binding::registerFunction(L, "newBox", lModelNewBox);
  lua_pop(L, 1);
}

void registerTextureKey(lua_State *L) {
  pushSnNamed(L, "TextureKey");
  Binding::registerInteger(L, "BaseColor",
                         static_cast<lua_Integer>(TextureKey::BaseColor));
  Binding::registerInteger(L, "Normal",
                         static_cast<lua_Integer>(TextureKey::Normal));
  Binding::registerInteger(
      L, "DiffuseRoughness",
      static_cast<lua_Integer>(TextureKey::DiffuseRoughness));
  Binding::registerInteger(L, "Metalness",
                         static_cast<lua_Integer>(TextureKey::Metalness));
  Binding::registerInteger(L, "Emissive",
                         static_cast<lua_Integer>(TextureKey::Emissive));
  Binding::registerInteger(L, "LightMap",
                         static_cast<lua_Integer>(TextureKey::LightMap));
  lua_pop(L, 1);
}
} // namespace sinen
