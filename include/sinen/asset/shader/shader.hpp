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
  void LoadDefaultVertexShader();
  void LoadDefaultVertexInstanceShader();
  void LoadDefaultFragmentShader();
  void LoadVertexShader(std::string_view vertex_shader, int numUniformData);
  void LoadFragmentShader(std::string_view fragment_shader, int numUniformData);

  void CompileAndLoadVertexShader(std::string_view vertex_shader,
                                  int numUniformData);
  void CompileAndLoadFragmentShader(std::string_view fragment_shader,
                                    int numUniformData);

private:
  friend class GraphicsPipeline2D;
  friend class GraphicsPipeline3D;
  px::Ptr<px::Shader> shader;
};
} // namespace sinen
#endif // !SINEN_SHADER_HPP
