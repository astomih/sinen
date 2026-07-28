#include "luaapi.hpp"
#include <core/allocator/global_allocator.hpp>
#include <core/thread/load_context.hpp>
#include <graphics/graphics_pipeline.hpp>

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
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "setVertexShader", lPipelineSetVertexShader);
  Binding::registerFunction(L, "setFragmentShader", lPipelineSetFragmentShader);
  Binding::registerFunction(L, "setEnableDepthTest",
                            lPipelineSetEnableDepthTest);
  Binding::registerFunction(L, "setEnableDepthWrite",
                            lPipelineSetEnableDepthWrite);
  Binding::registerFunction(L, "setEnableInstanced",
                            lPipelineSetEnableInstanced);
  Binding::registerFunction(L, "setEnableAnimation",
                            lPipelineSetEnableAnimation);
  Binding::registerFunction(L, "setEnableTangent", lPipelineSetEnableTangent);
  Binding::registerFunction(L, "build", lPipelineBuild);
  lua_pop(L, 1);

  pushSnNamed(L, "GraphicsPipeline");
  Binding::registerFunction(L, "new", lPipelineNew);
  lua_pop(L, 1);
}
} // namespace sinen
