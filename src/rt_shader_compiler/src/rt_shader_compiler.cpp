#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
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

  std::string input{};
  input += "slangc ";
  switch (lang) {
  case Language::SPIRV:
    input += "-profile glsl_450 ";
    input += "-target spirv ";
    break;
  default:
    break;
  }
  switch (type) {
  case Type::VERTEX:
    input += " -entry VSMain ";
    break;
  case Type::FRAGMENT:
    input += " -entry FSMain ";
    break;
  case Type::COMPUTE:
    input += " -entry CSMain ";
    break;
  }
  input += std::string(source);

  std::string tempSave = ".temp/";
  // now date/time based name
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  tempSave += std::to_string(in_time_t);
  tempSave += ".spv";

  input += " -o " + tempSave;

  // if temp directory does not exist, create it
  if (!std::filesystem::exists(".temp"))
    std::filesystem::create_directory(".temp");

  std::system(input.c_str());

  std::ifstream file(tempSave, std::ios::binary);
  if (file.is_open()) {
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    file.close();
    return buffer;
  }

  // auto *pipeOut = popen(input.c_str(), "r");
  // if (!pipeOut) {
  //   throw std::runtime_error("Failed to open pipe for writing");
  // }
  //
  // int ch;
  // while ((ch = fgetc(pipeOut)) != EOF) {
  //   buffer.push_back(ch);
  // }
  // pclose(pipeOut);

  std::vector<char> buffer{};
  return buffer;

#endif
}
} // namespace rsc