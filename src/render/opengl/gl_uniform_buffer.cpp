#include "gl_uniform_buffer.hpp"
#if defined(EMSCRIPTEN) || defined(MOBILE)
#include <GLES3/gl3.h>
#endif
namespace sinen {

void gl_uniform_buffer::create(const GLuint &blockIndex, const GLuint &size,
                               const void *data, const GLsizeiptr &offset,
                               const GLenum &usage) {

  GLuint BIB = 0; // blockIndexBuffer
  glGenBuffers(1, &BIB);
  glBindBuffer(GL_UNIFORM_BUFFER, BIB);
  glBufferData(GL_UNIFORM_BUFFER, size, data, usage);
  block_index_buffers.emplace(blockIndex, BIB);
}
void gl_uniform_buffer::update(const GLuint &blockIndex, const size_t &size,
                               const void *data, const GLsizeiptr &offset) {
  auto BIB = block_index_buffers[blockIndex];
  glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}
void gl_uniform_buffer::bind(const GLuint &program, const GLuint &blockIndex) {
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, block_index_buffers[blockIndex]);
  glUniformBlockBinding(program, blockIndex, 1);
}
void gl_uniform_buffer::destroy() {
  for (auto &BIB : block_index_buffers) {
    glDeleteBuffers(1, &BIB.second);
  }
}
} // namespace sinen