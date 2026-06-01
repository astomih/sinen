#include <core/allocator/global_allocator.hpp>
#include <core/thread/load_context.hpp>
#include <graphics/graphics_pipeline.hpp>
#include <script/luaapi.hpp>

namespace sinen {
static int lPipelineNew(lua_State *L) {
  udPushPtr<GraphicsPipeline>(L, makePtr<GraphicsPipeline>());
  return 1;
}
static int lPipelineSetVertexShader(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  auto &s = udPtr<Shader>(L, 2);
  p->setVertexShader(*s);
  return 0;
}
static int lPipelineSetFragmentShader(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  auto &s = udPtr<Shader>(L, 2);
  p->setFragmentShader(*s);
  return 0;
}
static int lPipelineSetEnableDepthTest(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableDepthTest(enable);
  return 0;
}
static int lPipelineSetEnableDepthWrite(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableDepthWrite(enable);
  return 0;
}
static int lPipelineSetEnableInstanced(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableInstanced(enable);
  return 0;
}
static int lPipelineSetEnableAnimation(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableAnimation(enable);
  return 0;
}
static int lPipelineSetEnableTangent(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableTangent(enable);
  return 0;
}
static int lPipelineBuild(lua_State *L) {
  udPtr<GraphicsPipeline>(L, 1)->build();
  return 0;
}
void registerPipeline(lua_State *L) {
  luaL_newmetatable(L, GraphicsPipeline::metaTableName());
  luaPushcfunction2(L, udPtrGc<GraphicsPipeline>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lPipelineSetVertexShader);
  lua_setfield(L, -2, "setVertexShader");
  luaPushcfunction2(L, lPipelineSetFragmentShader);
  lua_setfield(L, -2, "setFragmentShader");
  luaPushcfunction2(L, lPipelineSetEnableDepthTest);
  lua_setfield(L, -2, "setEnableDepthTest");
  luaPushcfunction2(L, lPipelineSetEnableDepthWrite);
  lua_setfield(L, -2, "setEnableDepthWrite");
  luaPushcfunction2(L, lPipelineSetEnableInstanced);
  lua_setfield(L, -2, "setEnableInstanced");
  luaPushcfunction2(L, lPipelineSetEnableAnimation);
  lua_setfield(L, -2, "setEnableAnimation");
  luaPushcfunction2(L, lPipelineSetEnableTangent);
  lua_setfield(L, -2, "setEnableTangent");
  luaPushcfunction2(L, lPipelineBuild);
  lua_setfield(L, -2, "build");
  lua_pop(L, 1);

  pushSnNamed(L, "GraphicsPipeline");
  luaPushcfunction2(L, lPipelineNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
