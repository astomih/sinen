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
void registerRenderTexture(luau::table &sn) {
  auto renderTexture =
      engineClass<Ptr<RenderTexture>>(sn, RenderTexture::metaTableName());

  renderTexture.method<lRenderTextureCreate>("create");

  renderTexture.static_function<lRenderTextureNew>("new");
}
} // namespace sinen
