#include "builtin_shader.hpp"
#include <graphics/graphics.hpp>

#include "default/cubemap.frag.spv.hpp"
#include "default/cubemap.vert.spv.hpp"
#include "default/shader.frag.spv.hpp"
#include "default/shader.vert.spv.hpp"
#include "default/shader_instance.vert.spv.hpp"

namespace sinen {
static Shader defaultVS;
static Shader defaultInstancedVS;
static Shader defaultFS;
static Shader cubemapVS;
static Shader cubemapFS;

bool BuiltinShader::initialize() {
  {

    auto *allocator = GlobalAllocator::get();
    auto device = Graphics::getDevice();

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
    defaultVS = Shader(device->createShader(vsInfo));
  }
  {
    auto *allocator = GlobalAllocator::get();
    auto device = Graphics::getDevice();

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
    defaultInstancedVS = Shader(device->createShader(vsInfo));
  }
  {
    auto *allocator = GlobalAllocator::get();
    auto device = Graphics::getDevice();

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
    defaultFS = Shader(device->createShader(fsInfo));
  }
  {
    auto *allocator = GlobalAllocator::get();
    auto device = Graphics::getDevice();

    rhi::Shader::CreateInfo vsInfo{};
    vsInfo.allocator = allocator;
    vsInfo.size = cubemap_vert_spv_len;
    vsInfo.data = cubemap_vert_spv;
    vsInfo.entrypoint = "main";
    vsInfo.format = rhi::ShaderFormat::SPIRV;
    vsInfo.stage = rhi::ShaderStage::Vertex;
    vsInfo.numSamplers = 0;
    vsInfo.numStorageBuffers = 0;
    vsInfo.numStorageTextures = 0;
    vsInfo.numUniformBuffers = 1; // only one uniform buffer for vertex shader
    cubemapVS = Shader(device->createShader(vsInfo));
  }
  {
    auto *allocator = GlobalAllocator::get();
    auto device = Graphics::getDevice();

    rhi::Shader::CreateInfo fsInfo{};
    fsInfo.allocator = allocator;
    fsInfo.size = cubemap_frag_spv_len;
    fsInfo.data = cubemap_frag_spv;
    fsInfo.entrypoint = "main";
    fsInfo.format = rhi::ShaderFormat::SPIRV;
    fsInfo.stage = rhi::ShaderStage::Fragment;
    fsInfo.numSamplers = 1; // one sampler for fragment shader
    fsInfo.numStorageBuffers = 0;
    fsInfo.numStorageTextures = 0;
    fsInfo.numUniformBuffers = 1; // only one uniform buffer for fragment shader
    cubemapFS = Shader(device->createShader(fsInfo));
  }

  return true;
}
void BuiltinShader::shutdown() {}

Shader BuiltinShader::getDefaultVS() { return defaultVS; }
Shader BuiltinShader::getDefaultInstancedVS() { return defaultInstancedVS; }
Shader BuiltinShader::getDefaultFS() { return defaultFS; }
Shader BuiltinShader::getCubemapVS() { return cubemapVS; }
Shader BuiltinShader::getCubemapFS() { return cubemapFS; }
} // namespace sinen