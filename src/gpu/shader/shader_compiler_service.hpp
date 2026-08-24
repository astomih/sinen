#ifndef SINEN_SHADER_COMPILER_SERVICE_HPP
#define SINEN_SHADER_COMPILER_SERVICE_HPP

#include <core/data/array.hpp>
#include <core/data/string.hpp>
#include <gpu/shader/shader_compiler_plugin.h>
#include <gpu/shader/shader_format.hpp>
#include <gpu/shader/shader_stage.hpp>

#include <cstdint>
#include <string>

namespace sinen {

struct CompiledShader {
  struct ResourceBinding {
    String name;
    uint32_t slot = 0;
  };

  Array<char> code;
  uint32_t numUniformBuffers = 0;
  uint32_t numCombinedSamplers = 0;
  uint32_t numStorageBuffers = 0;
  uint32_t numStorageTextures = 0;
  Array<ResourceBinding> uniformBuffers;
  Array<ResourceBinding> textures;
};

bool registerShaderCompilerPlugin(const sinen_shader_compiler_api *api,
                                  std::string &error);
void unregisterShaderCompilerPlugin(
    const sinen_shader_compiler_api *api = nullptr);

bool compileShaderSource(StringView moduleName, StringView modulePath,
                         StringView source, ShaderStage stage,
                         ShaderFormat format, CompiledShader &compiled,
                         std::string &error);

} // namespace sinen

#endif // SINEN_SHADER_COMPILER_SERVICE_HPP
