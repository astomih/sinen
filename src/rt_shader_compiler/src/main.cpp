#include <cstdio>
#include <fstream>
#include <iostream>
#include <rt_shader_compiler.hpp>

int main() {
  rsc::ShaderCompiler compiler;
  // load
  std::ifstream file("test.slang");
  std::string source((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());

  auto spirv = compiler.compile(source, rsc::ShaderCompiler::Type::VERTEX,
                                rsc::ShaderCompiler::Language::SPIRV);

  for (auto c : spirv) {
    printf("%c", c);
  }
  return 0;
}