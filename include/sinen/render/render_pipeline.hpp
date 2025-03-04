#ifndef SINEN_RENDER_PIPELINE_HPP
#define SINEN_RENDER_PIPELINE_HPP
#include <memory>

#include "../shader/shader.hpp"

namespace sinen {
class RenderPipeline {
public:
  RenderPipeline() = default;

  void set_vertex_shader(const Shader &shader);
  void set_fragment_shader(const Shader &shader);

private:
  std::shared_ptr<class RenderPipelineImpl> impl;
};
} // namespace sinen

#endif // !SINEN_RENDER_PIPELINE_HPP