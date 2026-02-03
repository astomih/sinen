#include <core/allocator/global_allocator.hpp>
#include <graphics/graphics.hpp>
#include <graphics/texture/render_texture.hpp>
#include <script/luaapi.hpp>

namespace sinen {

RenderTexture::RenderTexture() : texture(nullptr) {}

void RenderTexture::create(int width, int height) {
  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
  gpu::Texture::CreateInfo info{};
  info.allocator = allocator;
  info.width = width;
  info.height = height;
  info.layerCountOrDepth = 1;
  info.format = Graphics::getDevice()->getSwapchainFormat();
  info.usage = gpu::TextureUsage::ColorTarget;
  info.numLevels = 1;
  info.sampleCount = gpu::SampleCount::x1;
  info.type = gpu::TextureType::Texture2D;
  texture = device->createTexture(info);
  info.format = gpu::TextureFormat::D32_FLOAT_S8_UINT;
  info.usage = gpu::TextureUsage::DepthStencilTarget;
  depthStencil = device->createTexture(info);

  this->width = width;
  this->height = height;
}

Ptr<gpu::Texture> RenderTexture::getTexture() const { return texture; }

Ptr<gpu::Texture> RenderTexture::getDepthStencil() const {
  return depthStencil;
}

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
