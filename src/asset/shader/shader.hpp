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
  void loadVertexShader(std::string_view vertex_shader, int numUniformData);
  void loadFragmentShader(std::string_view fragment_shader, int numUniformData);

  void compileAndLoadVertexShader(std::string_view vertex_shader);
  void compileAndLoadFragmentShader(std::string_view fragment_shader);

private:
  friend class GraphicsPipeline2D;
  friend class GraphicsPipeline;
  Ptr<rhi::Shader> shader;
};
} // namespace sinen
#endif // !SINEN_SHADER_HPP
