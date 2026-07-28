#include "luaapi.hpp"
#include <core/allocator/global_allocator.hpp>
#include <graphics/graphics.hpp>
#include <graphics/texture/render_texture.hpp>

namespace sinen {
static int lRenderTextureNew(lua_State *L) {
  udPushPtr<RenderTexture>(L, makePtr<RenderTexture>());
  return 1;
}
static int lRenderTextureCreate(lua_State *L) {
  auto &rt = udPtr<RenderTexture>(L, 1);
  int w = static_cast<int>(luaL_checkinteger(L, 2));
  int h = static_cast<int>(luaL_checkinteger(L, 3));
  rt->create(w, h);
  return 0;
}
void registerRenderTexture(lua_State *L) {
  luaL_newmetatable(L, RenderTexture::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "create", lRenderTextureCreate);
  lua_pop(L, 1);

  pushSnNamed(L, "RenderTexture");
  Binding::registerFunction(L, "new", lRenderTextureNew);
  lua_pop(L, 1);
}
} // namespace sinen
