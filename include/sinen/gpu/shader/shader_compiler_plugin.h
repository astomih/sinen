#ifndef SINEN_SHADER_COMPILER_PLUGIN_H
#define SINEN_SHADER_COMPILER_PLUGIN_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SINEN_SHADER_COMPILER_PLUGIN_API_VERSION 1u
#define SINEN_SHADER_COMPILER_PLUGIN_API_SYMBOL "sinen_shader_compiler_get_api"

#if defined(_WIN32) && defined(SINEN_SHADER_COMPILER_PLUGIN_BUILD)
#define SINEN_SHADER_COMPILER_PLUGIN_EXPORT __declspec(dllexport)
#elif defined(SINEN_SHADER_COMPILER_PLUGIN_BUILD) &&                           \
    (defined(__GNUC__) || defined(__clang__))
#define SINEN_SHADER_COMPILER_PLUGIN_EXPORT                                    \
  __attribute__((used, visibility("default")))
#else
#define SINEN_SHADER_COMPILER_PLUGIN_EXPORT
#endif

typedef struct sinen_shader_compiler_binding {
  const char *name;
  size_t name_size;
  uint32_t slot;
} sinen_shader_compiler_binding;

typedef struct sinen_shader_compiler_result {
  uint32_t struct_size;
  const void *code;
  size_t code_size;
  uint32_t num_uniform_buffers;
  uint32_t num_combined_samplers;
  uint32_t num_storage_buffers;
  uint32_t num_storage_textures;
  const sinen_shader_compiler_binding *uniform_buffers;
  size_t uniform_buffer_count;
  const sinen_shader_compiler_binding *textures;
  size_t texture_count;
  void *private_data;
  void (*release)(struct sinen_shader_compiler_result *result);
} sinen_shader_compiler_result;

typedef int (*sinen_shader_compiler_compile_fn)(
    const char *module_name, size_t module_name_size, const char *module_path,
    size_t module_path_size, const char *source, size_t source_size,
    uint32_t stage, uint32_t format, sinen_shader_compiler_result *result);

typedef struct sinen_shader_compiler_api {
  uint32_t api_version;
  uint32_t struct_size;
  sinen_shader_compiler_compile_fn compile;
} sinen_shader_compiler_api;

typedef const sinen_shader_compiler_api *(*sinen_shader_compiler_get_api_fn)(
    void);

SINEN_SHADER_COMPILER_PLUGIN_EXPORT const sinen_shader_compiler_api *
sinen_shader_compiler_get_api(void);

#ifdef __cplusplus
}
#endif

#endif // SINEN_SHADER_COMPILER_PLUGIN_H
