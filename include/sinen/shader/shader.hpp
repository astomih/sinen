#ifndef SINEN_SHADER_HPP
#define SINEN_SHADER_HPP
#include "shader_type.hpp"
#include <list>
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
  /**
   * @brief Additional shader parameters
   *
   */
  struct parameter {};
  struct parameter_t {
    std::shared_ptr<parameter> param;
    std::size_t size;
    std::size_t uniform_binding;
  };
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
  /**
   * @brief Set the vertex shader name
   *
   * @param vertex_shader
   */
  void set_vertex_shader(std::string_view vertex_shader);
  /**
   * @brief Set the fragment shader name
   *
   * @param fragment_shader
   */
  void set_fragment_shader(std::string_view fragment_shader);
  /**
   * @brief Get the vertex shader name
   *
   * @return const std::string&
   */
  const std::string &vertex_shader() const;
  /**
   * @brief Get the fragment shader name
   *
   * @return const std::string&
   */
  const std::string &fragment_shader() const;
  /**
   * @brief Add a parameter to the shader
   *
   * @tparam T
   * @tparam Args
   * @param size
   * @param uniform_binding
   * @param args
   */
  template <class T, class... Args>
  void add_parameter(std::size_t size, std::size_t uniform_binding,
                     Args &&...args) {
    parameter_t param;
    param.param = std::make_shared<T>(std::forward<Args>(args)...);
    param.size = size;
    param.uniform_binding = uniform_binding;
    m_parameters.push_back(param);
  }
  /**
   * @brief Add a parameter to the shader
   *
   * @param param parameter_t
   */
  void add_parameter(const parameter_t &param) {
    m_parameters.push_back(param);
  }
  /**
   * @brief Get the parameters
   *
   * @return const std::list<parameter_t>& parameters
   */
  const std::list<parameter_t> &parameters() const { return m_parameters; }

  bool operator==(const shader &info) const {
    return this->m_vert_name == info.m_vert_name &&
           this->m_frag_name == info.m_frag_name;
  }

private:
  std::list<parameter_t> m_parameters;
  std::string m_vert_name;
  std::string m_frag_name;
};
} // namespace sinen
#endif // !SINEN_SHADER_HPP
