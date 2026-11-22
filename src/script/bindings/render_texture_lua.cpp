#include "luaapi.hpp"
#include <core/allocator/global_allocator.hpp>
#include <graphics/graphics.hpp>
#include <graphics/texture/render_texture.hpp>

namespace sinen {
static auto lRenderTextureNew() { return makePtr<RenderTexture>(); }
static auto lRenderTextureCreate(Ptr<RenderTexture> &rt, int w, int h) {
  rt->create(w, h);
  return;
}
void registerRenderTexture(lua_State *L) {
  pushClassMetatable<Ptr<RenderTexture>>(L, RenderTexture::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luau::table(L, -1).function("create", lRenderTextureCreate);
  lua_pop(L, 1);

  pushSnNamed(L, "RenderTexture");
  luau::table(L, -1).function("new", lRenderTextureNew);
  lua_pop(L, 1);
}
} // namespace sinen
