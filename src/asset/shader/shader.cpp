// internal
#include "../../render/px_renderer.hpp"
#include "../../render/render_system.hpp"
#include "libs/paranoixa/library/SDL/include/SDL3/SDL_iostream.h"
#include <asset/shader/shader.hpp>
#include <core/io/asset_type.hpp>
#include <core/io/data_stream.hpp>

#include <SDL3/SDL.h>

#include "rt_shader_compiler.hpp"

namespace sinen {
void Shader::load_vertex_shader(std::string_view vertex_shader,
                                int numUniformData) {
  auto pxRenderer = RendererImpl::GetPxRenderer();
  auto *allocator = pxRenderer->GetAllocator();
  auto device = pxRenderer->GetDevice();

  SDL_IOStream *file = (SDL_IOStream *)DataStream::open_as_rwops(
      AssetType::Shader, vertex_shader);

  std::string vsStr =
      DataStream::open_as_string(AssetType::Shader, vertex_shader);

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
void Shader::load_fragment_shader(std::string_view fragment_shader,
                                  int numUniformData) {
  auto pxRenderer = RendererImpl::GetPxRenderer();
  auto *allocator = pxRenderer->GetAllocator();
  auto device = pxRenderer->GetDevice();

  std::string fsStr =
      DataStream::open_as_string(AssetType::Shader, fragment_shader);

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
void Shader::compile_and_load_vertex_shader(std::string_view vertex_shader,
                                            int numUniformData) {

  std::string vsStr =
      DataStream::convert_file_path(AssetType::Shader, vertex_shader);

  // TODO: add support for other languages
  rsc::ShaderCompiler compiler;
  auto spirv = compiler.compile(vsStr, rsc::ShaderCompiler::Type::VERTEX,
                                rsc::ShaderCompiler::Language::SPIRV);

  auto pxRenderer = RendererImpl::GetPxRenderer();
  auto *allocator = pxRenderer->GetAllocator();
  auto device = pxRenderer->GetDevice();

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
void Shader::compile_and_load_fragment_shader(std::string_view fragment_shader,
                                              int numUniformData) {
  std::string fsStr =
      DataStream::convert_file_path(AssetType::Shader, fragment_shader);
  rsc::ShaderCompiler compiler;
  auto spirv = compiler.compile(fsStr, rsc::ShaderCompiler::Type::FRAGMENT,
                                rsc::ShaderCompiler::Language::SPIRV);

  auto pxRenderer = RendererImpl::GetPxRenderer();
  auto *allocator = pxRenderer->GetAllocator();
  auto device = pxRenderer->GetDevice();

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
