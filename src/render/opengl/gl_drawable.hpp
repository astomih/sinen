#ifndef SINEN_GL_DRAWABLE_HPP
#define SINEN_GL_DRAWABLE_HPP
#include "gl_uniform_buffer.hpp"
#include <drawable/drawable.hpp>
#include <memory>

namespace sinen {
class gl_drawable {
public:
  std::shared_ptr<drawable> drawable_object;
  gl_uniform_buffer ubo;
};
} // namespace sinen

#endif // SINEN_GL_DRAWABLE_HPP
