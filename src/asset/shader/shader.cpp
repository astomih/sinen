// internal
#include "../../graphics/graphics_system.hpp"
#include <asset/shader/shader.hpp>
#include <core/io/asset_io.hpp>

#include "core/allocator/global_allocator.hpp"
#include "rt_shader_compiler.hpp"

#include "default/shader.frag.spv.hpp"
#include "default/shader.vert.spv.hpp"
#include "default/shader_instance.vert.spv.hpp"

// external
#include <SDL3/SDL.h>

namespace sinen {
void Shader::loadDefaultVertexShader() {
  auto *allocator = GlobalAllocator::get();
  auto device = GraphicsSystem::getDevice();

  rhi::Shader::CreateInfo vsInfo{};
  vsInfo.allocator = allocator;
  vsInfo.size = shader_vert_spv_len;
  vsInfo.data = shader_vert_spv;
  vsInfo.entrypoint = "main";
  vsInfo.format = rhi::ShaderFormat::SPIRV;
  vsInfo.stage = rhi::ShaderStage::Vertex;
  vsInfo.numSamplers = 0;
  vsInfo.numStorageBuffers = 0;
  vsInfo.numStorageTextures = 0;
  vsInfo.numUniformBuffers = 1; // only one uniform buffer for vertex shader
  shader = device->createShader(vsInfo);
}
void Shader::loadDefaultVertexInstanceShader() {
  auto *allocator = GlobalAllocator::get();
  auto device = GraphicsSystem::getDevice();

  rhi::Shader::CreateInfo vsInfo{};
  vsInfo.allocator = allocator;
  vsInfo.size = shader_instance_vert_spv_len;
  vsInfo.data = shader_instance_vert_spv;
  vsInfo.entrypoint = "main";
  vsInfo.format = rhi::ShaderFormat::SPIRV;
  vsInfo.stage = rhi::ShaderStage::Vertex;
  vsInfo.numSamplers = 0;
  vsInfo.numStorageBuffers = 0;
  vsInfo.numStorageTextures = 0;
  vsInfo.numUniformBuffers = 1; // only one uniform buffer for vertex shader
  shader = device->createShader(vsInfo);
}
void Shader::loadDefaultFragmentShader() {
  auto *allocator = GlobalAllocator::get();
  auto device = GraphicsSystem::getDevice();

  rhi::Shader::CreateInfo fsInfo{};
  fsInfo.allocator = allocator;
  fsInfo.size = shader_frag_spv_len;
  fsInfo.data = shader_frag_spv;
  fsInfo.entrypoint = "main";
  fsInfo.format = rhi::ShaderFormat::SPIRV;
  fsInfo.stage = rhi::ShaderStage::Fragment;
  fsInfo.numSamplers = 1; // one sampler for fragment shader
  fsInfo.numStorageBuffers = 0;
  fsInfo.numStorageTextures = 0;
  fsInfo.numUniformBuffers = 1; // only one uniform buffer for fragment shader
  shader = device->createShader(fsInfo);
}
void Shader::loadVertexShader(std::string_view vertex_shader,
                              int numUniformData) {
  auto *allocator = GlobalAllocator::get();
  auto device = GraphicsSystem::getDevice();

  SDL_IOStream *file = (SDL_IOStream *)AssetIO::openAsIOStream(vertex_shader);

  auto vsStr = AssetIO::openAsString(vertex_shader);

  rhi::Shader::CreateInfo vsInfo{};
  vsInfo.allocator = allocator;
  vsInfo.size = vsStr.size();
  vsInfo.data = vsStr.data();
  vsInfo.entrypoint = "main";
  vsInfo.format = rhi::ShaderFormat::SPIRV;
  vsInfo.stage = rhi::ShaderStage::Vertex;
  vsInfo.numSamplers = 0;
  vsInfo.numStorageBuffers = 0;
  vsInfo.numStorageTextures = 0;
  vsInfo.numUniformBuffers = numUniformData + 1;
  shader = device->createShader(vsInfo);
}
void Shader::loadFragmentShader(std::string_view fragment_shader,
                                int numUniformData) {
  auto *allocator = GlobalAllocator::get();
  auto device = GraphicsSystem::getDevice();

  auto fsStr = AssetIO::openAsString(fragment_shader);

  rhi::Shader::CreateInfo fsInfo{};
  fsInfo.allocator = allocator;
  fsInfo.size = fsStr.size();
  fsInfo.data = fsStr.data();
  fsInfo.entrypoint = "main";
  fsInfo.format = rhi::ShaderFormat::SPIRV;
  fsInfo.stage = rhi::ShaderStage::Fragment;
  fsInfo.numSamplers = 1;
  fsInfo.numStorageBuffers = 0;
  fsInfo.numStorageTextures = 0;
  fsInfo.numUniformBuffers = numUniformData + 1;
  shader = device->createShader(fsInfo);
}
void Shader::compileAndLoadVertexShader(std::string_view vertex_shader) {

  std::string vsStr = vertex_shader.data();

  // TODO: add support for other languages
  ShaderCompiler compiler;
  ShaderCompiler::ReflectionData reflectionData;
  auto spirv =
      compiler.compile(vsStr, ShaderCompiler::Type::VERTEX,
                       ShaderCompiler::Language::SPIRV, reflectionData);

  auto *allocator = GlobalAllocator::get();
  auto device = GraphicsSystem::getDevice();

  rhi::Shader::CreateInfo vsInfo{};
  vsInfo.allocator = allocator;
  vsInfo.size = spirv.size();
  vsInfo.data = spirv.data();
  vsInfo.entrypoint = "main";
  vsInfo.format = rhi::ShaderFormat::SPIRV;
  vsInfo.stage = rhi::ShaderStage::Vertex;
  vsInfo.numSamplers = 0;
  vsInfo.numStorageBuffers = 0;
  vsInfo.numStorageTextures = 0;
  vsInfo.numUniformBuffers = reflectionData.numUniformBuffers;
  shader = device->createShader(vsInfo);
}
void Shader::compileAndLoadFragmentShader(std::string_view fragment_shader) {

  std::string fsStr = fragment_shader.data();
  ShaderCompiler compiler;
  ShaderCompiler::ReflectionData reflectionData;
  auto spirv =
      compiler.compile(fsStr, ShaderCompiler::Type::FRAGMENT,
                       ShaderCompiler::Language::SPIRV, reflectionData);

  auto device = GraphicsSystem::getDevice();

  rhi::Shader::CreateInfo fsInfo{};
  fsInfo.allocator = GlobalAllocator::get();
  fsInfo.size = spirv.size();
  fsInfo.data = spirv.data();
  fsInfo.entrypoint = "main";
  fsInfo.format = rhi::ShaderFormat::SPIRV;
  fsInfo.stage = rhi::ShaderStage::Fragment;
  fsInfo.numSamplers = reflectionData.numCombinedSamplers;
  fsInfo.numStorageBuffers = 0;
  fsInfo.numStorageTextures = 0;
  fsInfo.numUniformBuffers = reflectionData.numUniformBuffers;
  shader = device->createShader(fsInfo);
}
} // namespace sinen
