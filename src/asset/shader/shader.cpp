// internal
#include <asset/shader/shader.hpp>
#include <graphics/graphics.hpp>
#include <platform/io/asset_io.hpp>

#include "core/allocator/global_allocator.hpp"
#include "rt_shader_compiler.hpp"

// external
#include <SDL3/SDL.h>

namespace sinen {
void Shader::load(StringView vertex_shader, ShaderStage stage,
                  int numUniformData) {
  auto *allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();

  SDL_IOStream *file = (SDL_IOStream *)AssetIO::openAsIOStream(vertex_shader);

  auto str = AssetIO::openAsString(vertex_shader);

  gpu::Shader::CreateInfo info{};
  info.allocator = allocator;
  info.size = str.size();
  info.data = str.data();
  info.entrypoint = "main";
  info.format = gpu::ShaderFormat::SPIRV;
  switch (stage) {
  case ShaderStage::Vertex:
    info.stage = gpu::ShaderStage::Vertex;
    break;
  case ShaderStage::Fragment:
    info.stage = gpu::ShaderStage::Fragment;
    break;
  case ShaderStage::Compute:
    info.stage = gpu::ShaderStage::Vertex; // TODO
    break;
  }
  info.numSamplers = stage==ShaderStage::Fragment?1:0;
  info.numStorageBuffers = 0;
  info.numStorageTextures = 0;
  info.numUniformBuffers = numUniformData + 1;
  shader = device->createShader(info);
}
void Shader::compileAndLoad(StringView name, ShaderStage stage) {

  String str = name.data();

  // TODO: add support for other languages
  ShaderCompiler compiler;
  ShaderCompiler::ReflectionData reflectionData;
  auto spirv = compiler.compile(str, stage, ShaderCompiler::Language::SPIRV,
                                reflectionData);

  auto *allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();

  gpu::Shader::CreateInfo info{};
  info.allocator = allocator;
  info.size = spirv.size();
  info.data = spirv.data();
  info.entrypoint = "main";
  info.format = gpu::ShaderFormat::SPIRV;
  switch (stage) {
  case ShaderStage::Vertex:
    info.stage = gpu::ShaderStage::Vertex;
    break;
  case ShaderStage::Fragment:
    info.stage = gpu::ShaderStage::Fragment;
    break;
  case ShaderStage::Compute:
    info.stage = gpu::ShaderStage::Vertex; // TODO
    break;
  }
  info.numSamplers =
      stage == ShaderStage::Fragment ? reflectionData.numCombinedSamplers : 0;
  info.numStorageBuffers = 0;
  info.numStorageTextures = 0;
  info.numUniformBuffers = reflectionData.numUniformBuffers;
  shader = device->createShader(info);
}
} // namespace sinen
