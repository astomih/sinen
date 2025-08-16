// internal
#include "../../graphics/graphics_system.hpp"
#include "libs/paranoixa/library/SDL/include/SDL3/SDL_iostream.h"
#include <asset/shader/shader.hpp>
#include <core/io/asset_type.hpp>
#include <core/io/data_stream.hpp>

#include <SDL3/SDL.h>

#include "rt_shader_compiler.hpp"

#include "default/shader.frag.spv.hpp"
#include "default/shader.vert.spv.hpp"
#include "default/shader_instance.vert.spv.hpp"

namespace sinen {
void Shader::LoadDefaultVertexShader() {
  auto *allocator = GraphicsSystem::GetAllocator();
  auto device = GraphicsSystem::GetDevice();

  px::Shader::CreateInfo vsInfo{};
  vsInfo.allocator = allocator;
  vsInfo.size = shader_vert_spv_len;
  vsInfo.data = shader_vert_spv;
  vsInfo.entrypoint = "main";
  vsInfo.format = px::ShaderFormat::SPIRV;
  vsInfo.stage = px::ShaderStage::Vertex;
  vsInfo.numSamplers = 0;
  vsInfo.numStorageBuffers = 0;
  vsInfo.numStorageTextures = 0;
  vsInfo.numUniformBuffers = 1; // only one uniform buffer for vertex shader
  shader = device->CreateShader(vsInfo);
}
void Shader::LoadDefaultVertexInstanceShader() {
  auto *allocator = GraphicsSystem::GetAllocator();
  auto device = GraphicsSystem::GetDevice();

  px::Shader::CreateInfo vsInfo{};
  vsInfo.allocator = allocator;
  vsInfo.size = shader_instance_vert_spv_len;
  vsInfo.data = shader_instance_vert_spv;
  vsInfo.entrypoint = "main";
  vsInfo.format = px::ShaderFormat::SPIRV;
  vsInfo.stage = px::ShaderStage::Vertex;
  vsInfo.numSamplers = 0;
  vsInfo.numStorageBuffers = 0;
  vsInfo.numStorageTextures = 0;
  vsInfo.numUniformBuffers = 1; // only one uniform buffer for vertex shader
  shader = device->CreateShader(vsInfo);
}
void Shader::LoadDefaultFragmentShader() {
  auto *allocator = GraphicsSystem::GetAllocator();
  auto device = GraphicsSystem::GetDevice();

  px::Shader::CreateInfo fsInfo{};
  fsInfo.allocator = allocator;
  fsInfo.size = shader_frag_spv_len;
  fsInfo.data = shader_frag_spv;
  fsInfo.entrypoint = "main";
  fsInfo.format = px::ShaderFormat::SPIRV;
  fsInfo.stage = px::ShaderStage::Fragment;
  fsInfo.numSamplers = 1; // one sampler for fragment shader
  fsInfo.numStorageBuffers = 0;
  fsInfo.numStorageTextures = 0;
  fsInfo.numUniformBuffers = 1; // only one uniform buffer for fragment shader
  shader = device->CreateShader(fsInfo);
}
void Shader::LoadVertexShader(std::string_view vertex_shader,
                              int numUniformData) {
  auto *allocator = GraphicsSystem::GetAllocator();
  auto device = GraphicsSystem::GetDevice();

  SDL_IOStream *file =
      (SDL_IOStream *)DataStream::OpenAsRWOps(AssetType::Shader, vertex_shader);

  std::string vsStr =
      DataStream::OpenAsString(AssetType::Shader, vertex_shader);

  px::Shader::CreateInfo vsInfo{};
  vsInfo.allocator = allocator;
  vsInfo.size = vsStr.size();
  vsInfo.data = vsStr.data();
  vsInfo.entrypoint = "main";
  vsInfo.format = px::ShaderFormat::SPIRV;
  vsInfo.stage = px::ShaderStage::Vertex;
  vsInfo.numSamplers = 0;
  vsInfo.numStorageBuffers = 0;
  vsInfo.numStorageTextures = 0;
  vsInfo.numUniformBuffers = numUniformData + 1;
  shader = device->CreateShader(vsInfo);
}
void Shader::LoadFragmentShader(std::string_view fragment_shader,
                                int numUniformData) {
  auto *allocator = GraphicsSystem::GetAllocator();
  auto device = GraphicsSystem::GetDevice();

  std::string fsStr =
      DataStream::OpenAsString(AssetType::Shader, fragment_shader);

  px::Shader::CreateInfo fsInfo{};
  fsInfo.allocator = allocator;
  fsInfo.size = fsStr.size();
  fsInfo.data = fsStr.data();
  fsInfo.entrypoint = "main";
  fsInfo.format = px::ShaderFormat::SPIRV;
  fsInfo.stage = px::ShaderStage::Fragment;
  fsInfo.numSamplers = 1;
  fsInfo.numStorageBuffers = 0;
  fsInfo.numStorageTextures = 0;
  fsInfo.numUniformBuffers = numUniformData + 1;
  shader = device->CreateShader(fsInfo);
}
void Shader::CompileAndLoadVertexShader(std::string_view vertex_shader,
                                        int numUniformData) {

  std::string vsStr = vertex_shader.data();

  // TODO: add support for other languages
  rsc::ShaderCompiler compiler;
  auto spirv = compiler.compile(vsStr, rsc::ShaderCompiler::Type::VERTEX,
                                rsc::ShaderCompiler::Language::SPIRV);

  auto *allocator = GraphicsSystem::GetAllocator();
  auto device = GraphicsSystem::GetDevice();

  px::Shader::CreateInfo vsInfo{};
  vsInfo.allocator = allocator;
  vsInfo.size = spirv.size();
  vsInfo.data = spirv.data();
  vsInfo.entrypoint = "main";
  vsInfo.format = px::ShaderFormat::SPIRV;
  vsInfo.stage = px::ShaderStage::Vertex;
  vsInfo.numSamplers = 0;
  vsInfo.numStorageBuffers = 0;
  vsInfo.numStorageTextures = 0;
  vsInfo.numUniformBuffers = numUniformData + 1;
  shader = device->CreateShader(vsInfo);
}
void Shader::CompileAndLoadFragmentShader(std::string_view fragment_shader,
                                          int numUniformData) {
  std::string fsStr = fragment_shader.data();
  rsc::ShaderCompiler compiler;
  auto spirv = compiler.compile(fsStr, rsc::ShaderCompiler::Type::FRAGMENT,
                                rsc::ShaderCompiler::Language::SPIRV);

  auto *allocator = GraphicsSystem::GetAllocator();
  auto device = GraphicsSystem::GetDevice();

  px::Shader::CreateInfo fsInfo{};
  fsInfo.allocator = allocator;
  fsInfo.size = spirv.size();
  fsInfo.data = spirv.data();
  fsInfo.entrypoint = "main";
  fsInfo.format = px::ShaderFormat::SPIRV;
  fsInfo.stage = px::ShaderStage::Fragment;
  fsInfo.numSamplers = 1;
  fsInfo.numStorageBuffers = 0;
  fsInfo.numStorageTextures = 0;
  fsInfo.numUniformBuffers = numUniformData + 1;
  shader = device->CreateShader(fsInfo);
}
} // namespace sinen
