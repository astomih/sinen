#ifndef SINEN_SHADER_HPP
#define SINEN_SHADER_HPP
#include <graphics/rhi/rhi.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace sinen {
/**
 * @brief Shader
 *
 */
class Shader {
public:
  void loadDefaultVertexShader();
  void loadDefaultVertexInstanceShader();
  void loadDefaultFragmentShader();
  void loadVertexShader(StringView vertex_shader, int numUniformData);
  void loadFragmentShader(StringView fragment_shader, int numUniformData);

  void compileAndLoadVertexShader(StringView vertex_shader);
  void compileAndLoadFragmentShader(StringView fragment_shader);

private:
  friend class GraphicsPipeline2D;
  friend class GraphicsPipeline;
  Ptr<rhi::Shader> shader;
};
} // namespace sinen
#endif // !SINEN_SHADER_HPP
