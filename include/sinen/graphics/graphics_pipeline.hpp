#ifndef SINEN_RENDER_PIPELINE_HPP
#define SINEN_RENDER_PIPELINE_HPP
#include "../asset/shader/shader.hpp"
#include "paranoixa/paranoixa.hpp"

namespace sinen {
class GraphicsPipeline2D {
public:
  GraphicsPipeline2D() = default;

  void setVertexShader(const Shader &shader);
  void setFragmentShader(const Shader &shader);

  void build();

  px::Ptr<px::GraphicsPipeline> get() { return pipeline; }

private:
  Shader vertexShader;
  Shader fragmentShader;
  px::Ptr<px::GraphicsPipeline> pipeline;
};

class GraphicsPipeline3D {
public:
  GraphicsPipeline3D() = default;

  void setVertexShader(const Shader &shader);
  void setVertexInstancedShader(const Shader &shader);
  void setFragmentShader(const Shader &shader);
  void setAnimation(bool animation);

  void build();

  px::Ptr<px::GraphicsPipeline> get() const { return pipeline; }
  px::Ptr<px::GraphicsPipeline> getInstanced() const { return instanced; }

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