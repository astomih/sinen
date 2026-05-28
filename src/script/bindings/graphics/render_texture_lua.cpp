#include <script/luaapi.hpp>
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
  luaPushcfunction2(L, udPtrGc<RenderTexture>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lRenderTextureCreate);
  lua_setfield(L, -2, "create");
  lua_pop(L, 1);

  pushSnNamed(L, "RenderTexture");
  luaPushcfunction2(L, lRenderTextureNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
