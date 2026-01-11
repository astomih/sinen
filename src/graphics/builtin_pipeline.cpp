#include "builtin_pipeline.hpp"
#include <asset/shader/builtin_shader.hpp>
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
} // namespace sinen