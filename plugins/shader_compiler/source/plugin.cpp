#include <shader_compiler/shader_compiler.hpp>

#include <gpu/shader/shader_compiler_plugin.h>
#include <script/sinen_lua_module.h>

#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct BindingStorage {
  std::vector<std::string> names;
  std::vector<sinen_shader_compiler_binding> bindings;
};

struct CompileStorage {
  std::vector<char> code;
  BindingStorage uniformBuffers;
  BindingStorage textures;
};

void releaseResult(sinen_shader_compiler_result *result) {
  if (!result) {
    return;
  }
  delete static_cast<CompileStorage *>(result->private_data);
  *result = {};
  result->struct_size = sizeof(*result);
}

void copyBindings(
    const sinen::Array<sinen::ShaderCompiler::ResourceBinding> &source,
    BindingStorage &destination) {
  destination.names.reserve(source.size());
  destination.bindings.reserve(source.size());
  for (const auto &binding : source) {
    destination.names.emplace_back(binding.name.data(), binding.name.size());
  }
  for (size_t i = 0; i < source.size(); ++i) {
    destination.bindings.push_back({destination.names[i].data(),
                                    destination.names[i].size(),
                                    source[i].slot});
  }
}

bool validStage(uint32_t stage) {
  switch (static_cast<sinen::ShaderStage>(stage)) {
  case sinen::ShaderStage::Vertex:
  case sinen::ShaderStage::Fragment:
  case sinen::ShaderStage::Compute:
  case sinen::ShaderStage::RayGeneration:
  case sinen::ShaderStage::AnyHit:
  case sinen::ShaderStage::ClosestHit:
  case sinen::ShaderStage::Miss:
  case sinen::ShaderStage::Intersection:
  case sinen::ShaderStage::Callable:
    return true;
  }
  return false;
}

bool validFormat(uint32_t format) {
  switch (static_cast<sinen::ShaderFormat>(format)) {
  case sinen::ShaderFormat::SPIRV:
  case sinen::ShaderFormat::WGSL:
  case sinen::ShaderFormat::DXBC:
  case sinen::ShaderFormat::DXIL:
  case sinen::ShaderFormat::SPIRV_1_3:
    return true;
  }
  return false;
}

int compile(const char *moduleName, size_t moduleNameSize,
            const char *modulePath, size_t modulePathSize, const char *source,
            size_t sourceSize, uint32_t stage, uint32_t format,
            sinen_shader_compiler_result *result) {
  if (!result || result->struct_size < sizeof(*result) ||
      (moduleNameSize > 0 && !moduleName) ||
      (modulePathSize > 0 && !modulePath) || (sourceSize > 0 && !source) ||
      !validStage(stage) || !validFormat(format)) {
    return 0;
  }

  try {
    std::string sourceCopy(source ? source : "", sourceSize);
    sinen::ShaderCompiler compiler;
    sinen::ShaderCompiler::ReflectionData reflection{};
    sinen::Array<char> code = compiler.compileSource(
        std::string_view(moduleName ? moduleName : "", moduleNameSize),
        std::string_view(modulePath ? modulePath : "", modulePathSize),
        sourceCopy, static_cast<sinen::ShaderStage>(stage),
        static_cast<sinen::ShaderFormat>(format), reflection);
    if (code.empty()) {
      return 0;
    }

    auto storage = std::make_unique<CompileStorage>();
    storage->code.assign(code.begin(), code.end());
    copyBindings(reflection.uniformBuffers, storage->uniformBuffers);
    copyBindings(reflection.textures, storage->textures);

    *result = {};
    result->struct_size = sizeof(*result);
    result->code = storage->code.data();
    result->code_size = storage->code.size();
    result->num_uniform_buffers = reflection.numUniformBuffers;
    result->num_combined_samplers = reflection.numCombinedSamplers;
    result->num_storage_buffers = reflection.numStorageBuffers;
    result->num_storage_textures = reflection.numStorageTextures;
    result->uniform_buffers = storage->uniformBuffers.bindings.data();
    result->uniform_buffer_count = storage->uniformBuffers.bindings.size();
    result->textures = storage->textures.bindings.data();
    result->texture_count = storage->textures.bindings.size();
    result->private_data = storage.release();
    result->release = releaseResult;
    return 1;
  } catch (...) {
    return 0;
  }
}

const sinen_shader_compiler_api api = {
    SINEN_SHADER_COMPILER_PLUGIN_API_VERSION,
    sizeof(sinen_shader_compiler_api),
    compile,
};

} // namespace

SINEN_SHADER_COMPILER_PLUGIN_EXPORT const sinen_shader_compiler_api *
sinen_shader_compiler_get_api(void) {
  return &api;
}

SINEN_LUA_MODULE_EXPORT uint32_t sinen_lua_module_get_api_version(void) {
  return SINEN_LUA_MODULE_API_VERSION;
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_open(lua_State *state) {
  (void)state;
  return 0;
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_close(lua_State *state) {
  (void)state;
  return 0;
}
