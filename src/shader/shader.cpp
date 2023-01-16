#include <shader/shader.hpp>

namespace sinen {
struct parameter_time : public shader::parameter {
  float time = 0;
};
shader::shader()
    : m_parameter_size(0), m_parameter(nullptr), m_vert_name("default"),
      m_frag_name("default") {
  //set_parameter<parameter_time>(sizeof(m_parameter_size));
}
shader::shader(std::string_view vertex_shader, std::string_view fragment_shader)
    : m_vert_name(vertex_shader), m_frag_name(fragment_shader) {}
void shader::set_vertex_shader(std::string_view vertex_shader) {
  m_vert_name = vertex_shader;
}
void shader::set_fragment_shader(std::string_view fragment_shader) {
  m_frag_name = fragment_shader;
}
const std::string &shader::vertex_shader() const { return m_vert_name; }
const std::string &shader::fragment_shader() const { return m_frag_name; }
} // namespace sinen
