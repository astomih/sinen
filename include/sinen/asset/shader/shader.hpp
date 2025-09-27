#ifndef SINEN_SHADER_HPP
#define SINEN_SHADER_HPP
#include <paranoixa/paranoixa.hpp>

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

  void compileAndLoadVertexShader(std::string_view vertex_shader,
                                  int numUniformData);
  void compileAndLoadFragmentShader(std::string_view fragment_shader,
                                    int numUniformData);

private:
  friend class GraphicsPipeline2D;
  friend class GraphicsPipeline3D;
  px::Ptr<px::Shader> shader;
};
} // namespace sinen
#endif // !SINEN_SHADER_HPP
