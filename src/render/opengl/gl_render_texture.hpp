#ifndef SINEN_GL_RENDER_TEXTURE_HPP
#define SINEN_GL_RENDER_TEXTURE_HPP
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#if defined(EMSCRIPTEN) || defined(ANDROID)
#include <GLES3/gl3.h>
#include <SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif
#include "gl_uniform_buffer.hpp"
namespace sinen {
class gl_render_texture {
public:
  void prepare();
  void bind();
  void destroy();
  uint32_t framebuffer;
  uint32_t rendertexture;
  uint32_t depthbuffer;
  gl_uniform_buffer ubo;
};
} // namespace sinen
#endif