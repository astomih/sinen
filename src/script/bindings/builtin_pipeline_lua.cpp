#include "luaapi.hpp"
#include <gpu/shader/builtin_shader.hpp>
#include <graphics/builtin_pipeline.hpp>

namespace sinen {
static int lBuiltinPipelineGetDefault3D(lua_State *L) {
  udPushPtr<GraphicsPipeline>(
      L, makePtr<GraphicsPipeline>(BuiltinPipeline::getDefault3D()));
  return 1;
}
static int lBuiltinPipelineGetInstanced3D(lua_State *L) {
  udPushPtr<GraphicsPipeline>(
      L, makePtr<GraphicsPipeline>(BuiltinPipeline::getInstanced3D()));
  return 1;
}
static int lBuiltinPipelineGetDefault2D(lua_State *L) {
  udPushPtr<GraphicsPipeline>(
      L, makePtr<GraphicsPipeline>(BuiltinPipeline::getDefault2D()));
  return 1;
}
static int lBuiltinPipelineGetFont2D(lua_State *L) {
  udPushPtr<GraphicsPipeline>(
      L, makePtr<GraphicsPipeline>(BuiltinPipeline::getFont2D()));
  return 1;
}
static int lBuiltinPipelineGetRect2D(lua_State *L) {
  udPushPtr<GraphicsPipeline>(
      L, makePtr<GraphicsPipeline>(BuiltinPipeline::getRect2D()));
  return 1;
}
static int lBuiltinPipelineGetCubemap(lua_State *L) {
  udPushPtr<GraphicsPipeline>(
      L, makePtr<GraphicsPipeline>(BuiltinPipeline::getCubemap()));
  return 1;
}
void registerBuiltinPipeline(lua_State *L) {
  pushSnNamed(L, "BuiltinPipeline");
  luau::table(L, -1).raw_function("getDefault3D", lBuiltinPipelineGetDefault3D);
  luau::table(L, -1).raw_function("getInstanced3D",
                                  lBuiltinPipelineGetInstanced3D);
  luau::table(L, -1).raw_function("getDefault2D", lBuiltinPipelineGetDefault2D);
  luau::table(L, -1).raw_function("getFont2D", lBuiltinPipelineGetFont2D);
  luau::table(L, -1).raw_function("getRect2D", lBuiltinPipelineGetRect2D);
  luau::table(L, -1).raw_function("getCubemap", lBuiltinPipelineGetCubemap);
  lua_pop(L, 1);
}
} // namespace sinen
