#include "gl_shader.hpp"
#include "io/dstream.hpp"
#include <SDL_image.h>
#include <fstream>
#include <iostream>
#include <logger/logger.hpp>
#include <sstream>

#if defined(EMSCRIPTEN) || defined(MOBILE)
#include <GLES3/gl3.h>
#endif

namespace nen {

gl_shader::gl_shader() : mShaderProgram(0), mVertexShader(0), mFragShader(0) {}

bool gl_shader::load(const std::string &vertName, const std::string &fragName) {
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
  return IsValidProgram();
}

void gl_shader::unload() {
  // Delete the program/shaders
  glDeleteProgram(mShaderProgram);
  glDeleteShader(mVertexShader);
  glDeleteShader(mFragShader);
}

void gl_shader::active(const GLuint &blockIndex) {
  // Set this program as the active one
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, blockIndexBuffers[blockIndex]);
  glUniformBlockBinding(mShaderProgram, blockIndex, 1);
  glUseProgram(mShaderProgram);
}

void gl_shader::disable() { glDisable(mShaderProgram); }
bool gl_shader::CompileShader(const std::string &fileName, GLenum shaderType,
                              GLuint &outShader) {
  std::string contents;
#if defined EMSCRIPTEN || defined MOBILE
  contents = "#version 300 es\n";
#else
  contents = "#version 330 core\n";
#endif
  contents += dstream::open_as_string(asset_type::gl_shader, fileName);
  const char *contentsChar = contents.c_str();

  // Create a shader of the specified type
  outShader = glCreateShader(shaderType);
  // Set the source characters and try to compile
  glShaderSource(outShader, 1, &(contentsChar), nullptr);
  glCompileShader(outShader);
  return IsCompiled(outShader);
}

bool gl_shader::IsCompiled(GLuint shader) {
  GLint status;
  // Query the compile status
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE) {
    char buffer[512];
    memset(buffer, 0, 512);
    glGetShaderInfoLog(shader, 511, nullptr, buffer);
    logger::error("Shader compilation failed: %s", buffer);
    return false;
  }

  return true;
}

bool gl_shader::IsValidProgram() {
  GLint status;
  // Query the link status
  glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    char buffer[512];
    memset(buffer, 0, 512);
    glGetProgramInfoLog(mShaderProgram, 511, nullptr, buffer);
    return false;
  }

  return true;
}

bool gl_shader::create_ubo(const GLuint &blockIndex, const size_t &size,
                           const void *data) {
  GLuint BIB = 0; // blockIndexBuffer
  glGenBuffers(1, &BIB);
  glBindBuffer(GL_UNIFORM_BUFFER, BIB);
  glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
  glUniformBlockBinding(mShaderProgram, blockIndex, 5);
  blockIndexBuffers.emplace(blockIndex, BIB);
  return true;
}

void gl_shader::update_ubo(const GLuint &blockIndex, const size_t &size,
                           const void *data, const GLsizeiptr &offset) {
  auto BIB = blockIndexBuffers[blockIndex];
  glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}

} // namespace nen
