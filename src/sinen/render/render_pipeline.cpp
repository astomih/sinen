#include <render/render_pipeline.hpp>

#include <paranoixa/paranoixa.hpp>

namespace sinen {

class RenderPipelineImpl {

public:
  void set_vertex_shader(const Shader &shader) { vertexShader = shader; }

  void set_fragment_shader(const Shader &shader) { fragmentShader = shader; }

private:
  Shader vertexShader;
  Shader fragmentShader;
  px::Ptr<px::GraphicsPipeline> pipeline;
};

void RenderPipeline::set_vertex_shader(const Shader &shader) {
  impl->set_vertex_shader(shader);
}
void RenderPipeline::set_fragment_shader(const Shader &shader) {
  impl->set_fragment_shader(shader);
}
} // namespace sinen