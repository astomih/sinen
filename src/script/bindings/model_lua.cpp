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


namespace sinen {
static int lModelNew(lua_State *L) {
  udPushPtr<Model>(L, makePtr<Model>());
  return 1;
}
static int lModelLoad(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  if (lua_isstring(L, 2)) {
    const char *path = luaL_checkstring(L, 2);
    m->load(StringView(path));
    return 0;
  }
  auto &buf = udValue<Buffer>(L, 2);
  m->load(buf);
  return 0;
}
static int lModelGetAabb(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  udNewOwned<AABB>(L, m->getAABB());
  return 1;
}
static int lModelLoadSprite(lua_State *L) {
  udPtr<Model>(L, 1)->loadSprite();
  return 0;
}
static int lModelLoadBox(lua_State *L) {
  udPtr<Model>(L, 1)->loadBox();
  return 0;
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
  luaPushcfunction2(L, lModelLoad);
  lua_setfield(L, -2, "load");
  luaPushcfunction2(L, lModelGetAabb);
  lua_setfield(L, -2, "getAABB");
  luaPushcfunction2(L, lModelLoadSprite);
  lua_setfield(L, -2, "loadSprite");
  luaPushcfunction2(L, lModelLoadBox);
  lua_setfield(L, -2, "loadBox");
  luaPushcfunction2(L, lModelGetBoneUniformBuffer);
  lua_setfield(L, -2, "getBoneUniformBuffer");
  luaPushcfunction2(L, lModelPlay);
  lua_setfield(L, -2, "play");
  luaPushcfunction2(L, lModelUpdate);
  lua_setfield(L, -2, "update");
  luaPushcfunction2(L, lModelHasTexture);
  lua_setfield(L, -2, "hasTexture");
  luaPushcfunction2(L, lModelGetTexture);
  lua_setfield(L, -2, "getTexture");
  luaPushcfunction2(L, lModelSetTexture);
  lua_setfield(L, -2, "setTexture");
  lua_pop(L, 1);

  pushSnNamed(L, "Model");
  luaPushcfunction2(L, lModelNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

void registerTextureKey(lua_State *L) {
  pushSnNamed(L, "TextureKey");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::BaseColor));
  lua_setfield(L, -2, "BaseColor");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::Normal));
  lua_setfield(L, -2, "Normal");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::DiffuseRoughness));
  lua_setfield(L, -2, "DiffuseRoughness");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::Metalness));
  lua_setfield(L, -2, "Metalness");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::Emissive));
  lua_setfield(L, -2, "Emissive");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::LightMap));
  lua_setfield(L, -2, "LightMap");
  lua_pop(L, 1);
}
} // namespace sinen
