#ifndef GL_UNIFORM_BUFFER_HPP
#define GL_UNIFORM_BUFFER_HPP
#if defined(EMSCRIPTEN) || defined(ANDROID)
#include <SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif

#include <unordered_map>
namespace sinen {
class gl_uniform_buffer {
public:
  void create(const GLuint &size, const void *data,
              const GLsizeiptr &offset = 0,
              const GLenum &usage = GL_DYNAMIC_DRAW);
  void update(const size_t &size, const void *data, const GLsizeiptr &offset);
  void bind(const GLuint &program);
  void destroy();

private:
  GLuint ubo;
};
} // namespace sinen
#endif // GL_UNIFORM_BUFFER_HPP
