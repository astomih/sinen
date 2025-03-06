#include <array>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vector>

#include <rt_shader_compiler.hpp>

#ifdef _MSC_VER
#define popen _popen
#define pclose _pclose
#endif
namespace rsc {

std::vector<char> ShaderCompiler::compile(std::string_view source, Type type,
                                          Language lang) {
#ifndef __EMSCRIPTEN__
  std::vector<char> spirvBinary;

  std::string input{};
  input += "slangc ";
  switch (lang) {
  case Language::SPIRV:
    input += "-profile glsl_450";
    break;
  default:
    break;
  }
  switch (type) {
  case Type::VERTEX:
    input += " -entry vertexMain";
    break;
  case Type::FRAGMENT:
    input += " -entry fragmentMain";
    break;
  case Type::COMPUTE:
    input += " -entry computeMain";
    break;
  }
  input += "\"" + std::string(source) + "\"";

  auto *pipeOut = popen(input.c_str(), "r");
  if (!pipeOut) {
    throw std::runtime_error("Failed to open pipe for writing");
  }

  std::array<char, 2048> buffer;
  while (true) {
    size_t bytesRead = std::fread(buffer.data(), 1, buffer.size(), pipeOut);
    if (bytesRead == 0)
      break;
    spirvBinary.insert(spirvBinary.end(), buffer.data(),
                       buffer.data() + bytesRead);
  }
  pclose(pipeOut);

  return spirvBinary;
#endif
}
} // namespace rsc