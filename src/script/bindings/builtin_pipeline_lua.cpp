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
  Binding::registerFunction(L, "getDefault3D", lBuiltinPipelineGetDefault3D);
  Binding::registerFunction(L, "getInstanced3D",
                            lBuiltinPipelineGetInstanced3D);
  Binding::registerFunction(L, "getDefault2D", lBuiltinPipelineGetDefault2D);
  Binding::registerFunction(L, "getFont2D", lBuiltinPipelineGetFont2D);
  Binding::registerFunction(L, "getRect2D", lBuiltinPipelineGetRect2D);
  Binding::registerFunction(L, "getCubemap", lBuiltinPipelineGetCubemap);
  lua_pop(L, 1);
}
} // namespace sinen
