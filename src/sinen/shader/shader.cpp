// internal
#include "../render/render_system.hpp"
#include <shader/shader.hpp>

namespace sinen {
Shader::Shader()
    : m_parameter(nullptr), m_parameter_size(0), m_vert_name("default"),
      m_frag_name("default") {}
Shader::Shader(std::string_view vertex_shader, std::string_view fragment_shader)
    : m_parameter(nullptr), m_parameter_size(0), m_vert_name(vertex_shader),
      m_frag_name(fragment_shader) {}
void Shader::set_vertex_shader(std::string_view vertex_shader) {
  m_vert_name = vertex_shader;
}
void Shader::set_fragment_shader(std::string_view fragment_shader) {
  m_frag_name = fragment_shader;
}
const std::string &Shader::vertex_shader() const { return m_vert_name; }
const std::string &Shader::fragment_shader() const { return m_frag_name; }

void Shader::load() { RendererImpl::load_shader(*this); }
void Shader::unload() { RendererImpl::unload_shader(*this); }
} // namespace sinen
