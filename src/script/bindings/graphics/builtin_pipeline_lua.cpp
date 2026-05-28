#include <script/luaapi.hpp>
#include <graphics/builtin_pipeline.hpp>
#include <gpu/builtin_shader.hpp>

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
  luaPushcfunction2(L, lBuiltinPipelineGetDefault3D);
  lua_setfield(L, -2, "getDefault3D");
  luaPushcfunction2(L, lBuiltinPipelineGetInstanced3D);
  lua_setfield(L, -2, "getInstanced3D");
  luaPushcfunction2(L, lBuiltinPipelineGetDefault2D);
  lua_setfield(L, -2, "getDefault2D");
  luaPushcfunction2(L, lBuiltinPipelineGetFont2D);
  lua_setfield(L, -2, "getFont2D");
  luaPushcfunction2(L, lBuiltinPipelineGetRect2D);
  lua_setfield(L, -2, "getRect2D");
  luaPushcfunction2(L, lBuiltinPipelineGetCubemap);
  lua_setfield(L, -2, "getCubemap");
  lua_pop(L, 1);
}
} // namespace sinen
