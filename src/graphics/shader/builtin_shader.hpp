#ifndef SINEN_BUILTIN_SHADER_HPP
#define SINEN_BUILTIN_SHADER_HPP
#include "shader.hpp"
namespace sinen {
class BuiltinShader {
public:
  static bool initialize();
  static void shutdown();
  static Shader getDefaultVS();
  static Shader getDefaultInstancedVS();
  static Shader getDefaultFS();
  static Shader getRectFS();
  static Shader getCubemapVS();
  static Shader getCubemapFS();
};
} // namespace sinen
#endif
