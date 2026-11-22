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
void registerBuiltinPipeline(luau::table &sn) {
  auto builtinPipeline = sn.get_or_create_table("BuiltinPipeline");
  builtinPipeline.raw_function<lBuiltinPipelineGetDefault3D>("getDefault3D");
  builtinPipeline.raw_function<lBuiltinPipelineGetInstanced3D>(
      "getInstanced3D");
  builtinPipeline.raw_function<lBuiltinPipelineGetDefault2D>("getDefault2D");
  builtinPipeline.raw_function<lBuiltinPipelineGetFont2D>("getFont2D");
  builtinPipeline.raw_function<lBuiltinPipelineGetRect2D>("getRect2D");
  builtinPipeline.raw_function<lBuiltinPipelineGetCubemap>("getCubemap");
}
} // namespace sinen
