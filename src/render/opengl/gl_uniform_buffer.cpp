#include "gl_uniform_buffer.hpp"
#if defined(EMSCRIPTEN) || defined(ANDROID)
#define GL_GLEXT_PROTOTYPES
#include <GLES3/gl3.h>
#include <SDL_opengles2.h>
#endif
namespace sinen {

void gl_uniform_buffer::create(const GLuint &size, const void *data,
                               const GLsizeiptr &offset, const GLenum &usage) {

  glGenBuffers(1, &ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  glBufferData(GL_UNIFORM_BUFFER, size, data, usage);
}
void gl_uniform_buffer::update(const size_t &size, const void *data,
                               const GLsizeiptr &offset) {
  glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}
void gl_uniform_buffer::bind(const GLuint &program) {
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);
  glUniformBlockBinding(program, 0, 1);
}
void gl_uniform_buffer::destroy() { glDeleteBuffers(1, &ubo); }
} // namespace sinen
