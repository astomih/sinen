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
void registerPipeline(luau::table &sn) {
  auto graphicsPipeline =
      engineClass<Ptr<GraphicsPipeline>>(sn, GraphicsPipeline::metaTableName());

  graphicsPipeline.method<lPipelineSetVertexShader>("setVertexShader");
  graphicsPipeline.method<lPipelineSetFragmentShader>("setFragmentShader");
  graphicsPipeline.raw_method<lPipelineSetEnableDepthTest>(
      "setEnableDepthTest");
  graphicsPipeline.raw_method<lPipelineSetEnableDepthWrite>(
      "setEnableDepthWrite");
  graphicsPipeline.raw_method<lPipelineSetEnableInstanced>(
      "setEnableInstanced");
  graphicsPipeline.raw_method<lPipelineSetEnableAnimation>(
      "setEnableAnimation");
  graphicsPipeline.raw_method<lPipelineSetEnableTangent>("setEnableTangent");
  graphicsPipeline.raw_method<lPipelineBuild>("build");

  graphicsPipeline.static_function<lPipelineNew>("new");
}
} // namespace sinen
