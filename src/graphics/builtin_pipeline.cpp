#include "builtin_pipeline.hpp"
#include <graphics/shader/builtin_shader.hpp>
#include <script/luaapi.hpp>
namespace sinen {
static GraphicsPipeline default2D;
static GraphicsPipeline default3D;
static GraphicsPipeline instanced3D;
static GraphicsPipeline cubemap;
bool BuiltinPipeline::initialize() {
  Shader vs = BuiltinShader::getDefaultVS();
  Shader vsInstanced = BuiltinShader::getDefaultInstancedVS();
  Shader fs = BuiltinShader::getDefaultFS();
  Shader cubemapVS = BuiltinShader::getCubemapVS();
  Shader cubemapFS = BuiltinShader::getCubemapFS();

  default3D.setVertexShader(vs);
  default3D.setFragmentShader(fs);
  default3D.setEnableDepthTest(true);
  default3D.build();

  instanced3D.setVertexShader(vsInstanced);
  instanced3D.setFragmentShader(fs);
  instanced3D.setEnableInstanced(true);
  instanced3D.setEnableDepthTest(true);
  instanced3D.build();

  default2D.setVertexShader(vs);
  default2D.setFragmentShader(fs);
  default2D.setEnableDepthTest(false);
  default2D.build();

  cubemap.setVertexShader(cubemapVS);
  cubemap.setFragmentShader(cubemapFS);
  cubemap.setEnableDepthTest(false);
  cubemap.build();

  return true;
}
void BuiltinPipeline::shutdown() {}
GraphicsPipeline BuiltinPipeline::getDefault3D() { return default3D; }
GraphicsPipeline BuiltinPipeline::getInstanced3D() { return instanced3D; }
GraphicsPipeline BuiltinPipeline::getDefault2D() { return default2D; }
GraphicsPipeline BuiltinPipeline::getCubemap() { return cubemap; }

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
  luaPushcfunction2(L, lBuiltinPipelineGetCubemap);
  lua_setfield(L, -2, "getCubemap");
  lua_pop(L, 1);
}
} // namespace sinen
