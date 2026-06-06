#include "builtin_pipeline.hpp"
#include <gpu/shader/builtin_shader.hpp>

namespace sinen {
static GraphicsPipeline default2D;
static GraphicsPipeline font2D;
static GraphicsPipeline rect2D;
static GraphicsPipeline default3D;
static GraphicsPipeline instanced3D;
static GraphicsPipeline cubemap;
bool BuiltinPipeline::initialize() {
  Shader vs = BuiltinShader::getDefaultVS();
  Shader vsInstanced = BuiltinShader::getDefaultInstancedVS();
  Shader fs = BuiltinShader::getDefaultFS();
  Shader fontFS = BuiltinShader::getFontFS();
  Shader rectFS = BuiltinShader::getRectFS();
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

  font2D.setVertexShader(vs);
  font2D.setFragmentShader(fontFS);
  font2D.setEnableDepthTest(false);
  font2D.build();

  rect2D.setVertexShader(vs);
  rect2D.setFragmentShader(rectFS);
  rect2D.setEnableDepthTest(false);
  rect2D.build();

  cubemap.setVertexShader(cubemapVS);
  cubemap.setFragmentShader(cubemapFS);
  cubemap.setEnableDepthTest(true);
  cubemap.setEnableDepthWrite(false);
  cubemap.build();

  return true;
}
void BuiltinPipeline::shutdown() {
  default2D = GraphicsPipeline();
  font2D = GraphicsPipeline();
  rect2D = GraphicsPipeline();
  default3D = GraphicsPipeline();
  instanced3D = GraphicsPipeline();
  cubemap = GraphicsPipeline();
}
GraphicsPipeline BuiltinPipeline::getDefault3D() { return default3D; }
GraphicsPipeline BuiltinPipeline::getInstanced3D() { return instanced3D; }
GraphicsPipeline BuiltinPipeline::getDefault2D() { return default2D; }
GraphicsPipeline BuiltinPipeline::getFont2D() { return font2D; }
GraphicsPipeline BuiltinPipeline::getRect2D() { return rect2D; }
GraphicsPipeline BuiltinPipeline::getCubemap() { return cubemap; }

} // namespace sinen
