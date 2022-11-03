#ifndef GL_UNIFORM_BUFFER_HPP
#define GL_UNIFORM_BUFFER_HPP
#if defined(EMSCRIPTEN) || defined(MOBILE)
#include <SDL_opengles2.h>
#endif
#include <GL/glew.h>

#include <unordered_map>
namespace sinen {
class gl_uniform_buffer {
public:
  void create(const GLuint &blockIndex, const GLuint &size, const void *data,
              const GLsizeiptr &offset = 0,
              const GLenum &usage = GL_DYNAMIC_DRAW);
  void update(const GLuint &blockIndex, const size_t &size, const void *data,
              const GLsizeiptr &offset);
  void bind(const GLuint &program, const GLuint &blockIndex);
  void destroy();

private:
  std::unordered_map<GLuint, GLuint> block_index_buffers;
};
} // namespace sinen
#endif // GL_UNIFORM_BUFFER_HPP