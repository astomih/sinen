#include "shader_compiler_service.hpp"

#include <cstring>
#include <mutex>
#include <utility>

namespace sinen {
namespace {

std::mutex gCompilerMutex;
const sinen_shader_compiler_api *gCompilerApi = nullptr;

bool copyBindings(const sinen_shader_compiler_binding *source, size_t count,
                  Array<CompiledShader::ResourceBinding> &destination,
                  std::string &error) {
  if (count > 0 && !source) {
    error = "shader compiler returned a null resource binding array";
    return false;
  }

  destination.clear();
  destination.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    if (source[i].name_size > 0 && !source[i].name) {
      error = "shader compiler returned a null resource binding name";
      return false;
    }
    destination.push_back(
        {String(source[i].name ? source[i].name : "", source[i].name_size),
         source[i].slot});
  }
  return true;
}

class ResultGuard {
public:
  explicit ResultGuard(sinen_shader_compiler_result &value) : result(value) {}
  ~ResultGuard() {
    if (result.release) {
      result.release(&result);
    }
  }

private:
  sinen_shader_compiler_result &result;
};

} // namespace

bool registerShaderCompilerPlugin(const sinen_shader_compiler_api *api,
                                  std::string &error) {
  error.clear();
  if (!api) {
    error = "shader compiler plugin returned a null API";
    return false;
  }
  if (api->api_version != SINEN_SHADER_COMPILER_PLUGIN_API_VERSION) {
    error = "unsupported shader compiler plugin API version " +
            std::to_string(api->api_version);
    return false;
  }
  if (api->struct_size < sizeof(sinen_shader_compiler_api) || !api->compile) {
    error = "shader compiler plugin API is incomplete";
    return false;
  }

  std::scoped_lock lock(gCompilerMutex);
  if (gCompilerApi && gCompilerApi != api) {
    error = "a shader compiler plugin is already registered";
    return false;
  }
  gCompilerApi = api;
  return true;
}

void unregisterShaderCompilerPlugin(const sinen_shader_compiler_api *api) {
  std::scoped_lock lock(gCompilerMutex);
  if (!api || gCompilerApi == api) {
    gCompilerApi = nullptr;
  }
}

bool compileShaderSource(StringView moduleName, StringView modulePath,
                         StringView source, ShaderStage stage,
                         ShaderFormat format, CompiledShader &compiled,
                         std::string &error) {
  std::scoped_lock lock(gCompilerMutex);
  error.clear();
  compiled = {};
  if (!gCompilerApi) {
    error = "shader compiler plugin is not loaded";
    return false;
  }

  sinen_shader_compiler_result result{};
  result.struct_size = sizeof(result);
  ResultGuard resultGuard(result);
  if (!gCompilerApi->compile(moduleName.data(), moduleName.size(),
                             modulePath.data(), modulePath.size(),
                             source.data(), source.size(),
                             static_cast<uint32_t>(stage),
                             static_cast<uint32_t>(format), &result)) {
    error = "shader compiler plugin failed to compile the source";
    return false;
  }
  if (result.struct_size < sizeof(sinen_shader_compiler_result)) {
    error = "shader compiler plugin returned an incomplete result";
    return false;
  }
  if (result.code_size > 0 && !result.code) {
    error = "shader compiler plugin returned null shader code";
    return false;
  }

  compiled.code.resize(result.code_size);
  if (result.code_size > 0) {
    std::memcpy(compiled.code.data(), result.code, result.code_size);
  }
  compiled.numUniformBuffers = result.num_uniform_buffers;
  compiled.numCombinedSamplers = result.num_combined_samplers;
  compiled.numStorageBuffers = result.num_storage_buffers;
  compiled.numStorageTextures = result.num_storage_textures;
  if (!copyBindings(result.uniform_buffers, result.uniform_buffer_count,
                    compiled.uniformBuffers, error) ||
      !copyBindings(result.textures, result.texture_count, compiled.textures,
                    error)) {
    compiled = {};
    return false;
  }
  return true;
}

} // namespace sinen
