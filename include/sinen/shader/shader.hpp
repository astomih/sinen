#ifndef SINEN_SHADER_HPP
#define SINEN_SHADER_HPP
#include "shader_type.hpp"
#include <memory>
#include <string>
#include <string_view>

namespace sinen {
/**
 * @brief Shader
 *
 */
class shader {
public:
  struct parameter {};
  /**
   * @brief Construct a new shader object
   *
   */
  shader();
  /**
   * @brief Construct a new shader object
   *
   * @param vertex_shader vertex shader file name
   * @param fragment_shader fragment shader file name
   */
  shader(std::string_view vertex_shader, std::string_view fragment_shader);
  /**
   * @brief Destroy the shader object
   *
   */
  ~shader() = default;

  void set_vertex_shader(std::string_view vertex_shader);
  void set_fragment_shader(std::string_view fragment_shader);
  const std::string &vertex_shader() const;
  const std::string &fragment_shader() const;

  bool operator==(const shader &info) const {
    return this->vertName == info.vertName && this->fragName == info.fragName;
  }

private:
  std::shared_ptr<parameter> m_parameter;
  std::size_t m_parameter_size;
  std::size_t binding_point;
  std::string vertName;
  std::string fragName;
};
} // namespace sinen
#endif // !SINEN_SHADER_HPP
