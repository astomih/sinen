#ifndef SINEN_SHADER_HPP
#define SINEN_SHADER_HPP
#include "shader_type.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <vector>

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
   * @brief Set a parameter to the shader
   *
   * @tparam T Type
   * @tparam Args Constructor arguments of T
   * @param size Size of the parameter
   * @param args
   */
  template <class T, class... Args>
  void set_parameter(std::size_t size, Args &&...args) {
    m_parameter_size = size;
    m_parameter = std::make_shared<T>(std::forward<Args>(args)...);
  }
  /**
   * @brief Set a parameter to the shader
   *
   * @param param
   */
  void add_parameter(std::shared_ptr<parameter> param) { m_parameter = param; }
  /**
   * @brief Get the parameters
   *
   * @return std::shared_ptr<parameter> parameter
   */
  std::shared_ptr<parameter> get_parameter() const { return m_parameter; }
  const std::size_t get_parameter_size() const { return m_parameter_size; }

  bool operator==(const shader &info) const {
    return this->m_vert_name == info.m_vert_name &&
           this->m_frag_name == info.m_frag_name;
  }

private:
  std::shared_ptr<parameter> m_parameter;
  std::size_t m_parameter_size;
  std::string m_vert_name;
  std::string m_frag_name;
};
} // namespace sinen
#endif // !SINEN_SHADER_HPP
