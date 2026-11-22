#include "luaapi.hpp"
#include <core/allocator/global_allocator.hpp>
#include <core/thread/load_context.hpp>
#include <graphics/graphics_pipeline.hpp>

namespace sinen {
static auto lPipelineNew() { return makePtr<GraphicsPipeline>(); }
static auto lPipelineSetVertexShader(Ptr<GraphicsPipeline> &p, Ptr<Shader> &s) {
  p->setVertexShader(*s);
  return;
}
static auto lPipelineSetFragmentShader(Ptr<GraphicsPipeline> &p,
                                       Ptr<Shader> &s) {
  p->setFragmentShader(*s);
  return;
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
  pushClassMetatable<Ptr<GraphicsPipeline>>(L,
                                            GraphicsPipeline::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luau::table(L, -1).function("setVertexShader", lPipelineSetVertexShader);
  luau::table(L, -1).function("setFragmentShader", lPipelineSetFragmentShader);
  luau::table(L, -1).raw_function("setEnableDepthTest",
                                  lPipelineSetEnableDepthTest);
  luau::table(L, -1).raw_function("setEnableDepthWrite",
                                  lPipelineSetEnableDepthWrite);
  luau::table(L, -1).raw_function("setEnableInstanced",
                                  lPipelineSetEnableInstanced);
  luau::table(L, -1).raw_function("setEnableAnimation",
                                  lPipelineSetEnableAnimation);
  luau::table(L, -1).raw_function("setEnableTangent",
                                  lPipelineSetEnableTangent);
  luau::table(L, -1).raw_function("build", lPipelineBuild);
  lua_pop(L, 1);

  pushSnNamed(L, "GraphicsPipeline");
  luau::table(L, -1).function("new", lPipelineNew);
  lua_pop(L, 1);
}
} // namespace sinen
