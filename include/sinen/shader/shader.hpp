#ifndef SINEN_SHADER_HPP
#define SINEN_SHADER_HPP
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <paranoixa/paranoixa.hpp>

namespace sinen {
/**
 * @brief Shader
 *
 */
class Shader {
public:
  void load_vertex_shader(std::string_view vertex_shader, int numUniformData);
  void load_fragment_shader(std::string_view fragment_shader,
                            int numUniformData);

  void compile_and_load_vertex_shader(std::string_view vertex_shader,
                                      int numUniformData);
  void compile_and_load_fragment_shader(std::string_view fragment_shader,
                                        int numUniformData);

private:
  friend class RenderPipeline2D;
  friend class RenderPipeline3D;
  px::Ptr<px::Shader> shader;
};
} // namespace sinen
#endif // !SINEN_SHADER_HPP
