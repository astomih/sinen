#include "ShaderGL.h"
#include "IO/AssetReader.hpp"
#include <Nen.hpp>
#include <SDL_image.h>
#include <fstream>
#include <iostream>
#include <sstream>
#if defined(EMSCRIPTEN) || defined(MOBILE)
#include <GLES3/gl3.h>
#endif

namespace nen::gl {

ShaderGL::ShaderGL() : mShaderProgram(0), mVertexShader(0), mFragShader(0) {}

bool ShaderGL::Load(const std::string &vertName, const std::string &fragName) {
  // Compile vertex and pixel shaders
  if (!CompileShader(fragName, GL_FRAGMENT_SHADER, mFragShader) ||
      !CompileShader(vertName, GL_VERTEX_SHADER, mVertexShader)) {
    return false;
  }

  // Now create a shader program that links together the vertex/frag shaders
  mShaderProgram = glCreateProgram();
  glAttachShader(mShaderProgram, mVertexShader);
  glAttachShader(mShaderProgram, mFragShader);
  glLinkProgram(mShaderProgram);

  // Verify that the program linked successfully
  if (!IsValidProgram()) {
    return false;
  }

  return true;
}

void ShaderGL::Unload() {
  // Delete the program/shaders
  glDeleteProgram(mShaderProgram);
  glDeleteShader(mVertexShader);
  glDeleteShader(mFragShader);
}

void ShaderGL::SetActive(const GLuint &blockIndex) {
  // Set this program as the active one
  glUseProgram(mShaderProgram);
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, blockIndexBuffers[blockIndex]);
  glUniformBlockBinding(mShaderProgram, blockIndex, 1);
}

void ShaderGL::SetDisable() { glDisable(mShaderProgram); }
bool ShaderGL::CompileShader(const std::string &fileName, GLenum shaderType,
                             GLuint &outShader) {
  std::string contents;
#if defined EMSCRIPTEN || defined MOBILE
  contents = "#version 300 es\n";
#else
  contents = "#version 330 core\n";
#endif
  contents += asset_reader::LoadAsString(asset_type::gl_shader, fileName);
  const char *contentsChar = contents.c_str();

  // Create a shader of the specified type
  outShader = glCreateShader(shaderType);
  // Set the source characters and try to compile
  glShaderSource(outShader, 1, &(contentsChar), nullptr);
  glCompileShader(outShader);

  if (!IsCompiled(outShader)) {
    std::cout << "Failed to compile shader " << fileName << std::endl;
    return false;
  }
  return true;
}

bool ShaderGL::IsCompiled(GLuint shader) {
  GLint status;
  // Query the compile status
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE) {
    char buffer[512];
    memset(buffer, 0, 512);
    glGetShaderInfoLog(shader, 511, nullptr, buffer);
    std::cout << "GLSL Compile Failed: " << buffer << std::endl;
    return false;
  }

  return true;
}

bool ShaderGL::IsValidProgram() {
  GLint status;
  // Query the link status
  glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    char buffer[512];
    memset(buffer, 0, 512);
    glGetProgramInfoLog(mShaderProgram, 511, nullptr, buffer);
    std::cout << "GLSL Link Status: " << buffer << std::endl;
    return false;
  }

  return true;
}

bool ShaderGL::CreateUBO(const GLuint &blockIndex, const size_t &size,
                         const void *data) {
  GLuint BIB = 0; // blockIndexBuffer
  glGenBuffers(1, &BIB);
  glBindBuffer(GL_UNIFORM_BUFFER, BIB);
  glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
  glUniformBlockBinding(mShaderProgram, blockIndex, 1);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  blockIndexBuffers.emplace(blockIndex, BIB);
  return true;
}

void ShaderGL::UpdateUBO(const GLuint &blockIndex, const size_t &size,
                         const void *data, const GLsizeiptr &offset) {
  auto BIB = blockIndexBuffers[blockIndex];

  glBindBuffer(GL_UNIFORM_BUFFER, BIB);
  glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

} // namespace nen::gl
