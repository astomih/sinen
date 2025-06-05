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
  void load_default_vertex_shader();
  void load_default_vertex_instance_shader();
  void load_default_fragment_shader();
  void load_vertex_shader(std::string_view vertex_shader, int numUniformData);
  void load_fragment_shader(std::string_view fragment_shader,
                            int numUniformData);

  void compile_and_load_vertex_shader(std::string_view vertex_shader,
                                      int numUniformData);
  void compile_and_load_fragment_shader(std::string_view fragment_shader,
                                        int numUniformData);

private:
  friend class GraphicsPipeline2D;
  friend class GraphicsPipeline3D;
  px::Ptr<px::Shader> shader;
};
} // namespace sinen
#endif // !SINEN_SHADER_HPP
