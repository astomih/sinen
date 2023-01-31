#include "gl_render_texture.hpp"
#include "gl_renderer.hpp"
#include <logger/logger.hpp>
#include <window/window.hpp>
namespace sinen {
void gl_render_texture::prepare() {
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glGenTextures(1, &rendertexture);
  glBindTexture(GL_TEXTURE_2D, rendertexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window::size().x, window::size().y, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glGenRenderbuffers(1, &depthbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, window::size().x,
                        window::size().y);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, depthbuffer);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         rendertexture, 0);
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    logger::error("failed to create framebuffer");
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  gl_shader_parameter param{};
  ubo.create(0, sizeof(gl_shader_parameter), &param);
}
void gl_render_texture::bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}
void gl_render_texture::destroy() {
  glDeleteFramebuffers(1, &framebuffer);
  glDeleteTextures(1, &rendertexture);
  glDeleteRenderbuffers(1, &depthbuffer);
}
} // namespace sinen
