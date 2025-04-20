#ifndef SINEN_RENDER_PIPELINE_HPP
#define SINEN_RENDER_PIPELINE_HPP
#include "../asset/shader/shader.hpp"
#include "paranoixa/paranoixa.hpp"

namespace sinen {
class RenderPipeline2D {
public:
  RenderPipeline2D() = default;

  void set_vertex_shader(const Shader &shader);
  void set_fragment_shader(const Shader &shader);

  void build();

  px::Ptr<px::GraphicsPipeline> get() { return pipeline; }

private:
  Shader vertexShader;
  Shader fragmentShader;
  px::Ptr<px::GraphicsPipeline> pipeline;
};

class RenderPipeline3D {
public:
  RenderPipeline3D() = default;

  void set_vertex_shader(const Shader &shader);
  void set_vertex_instanced_shader(const Shader &shader);
  void set_fragment_shader(const Shader &shader);
  void set_animation(bool animation);

  void build();

  px::Ptr<px::GraphicsPipeline> get() const { return pipeline; }
  px::Ptr<px::GraphicsPipeline> get_instanced() const { return instanced; }

private:
  Shader vertexShader;
  Shader instancedShader;
  Shader fragmentShader;
  px::Ptr<px::GraphicsPipeline> pipeline;
  px::Ptr<px::GraphicsPipeline> instanced;
  bool isAnimation = false;
};
} // namespace sinen

#endif // !SINEN_RENDER_PIPELINE_HPP