#include <shader/shader.hpp>

namespace sinen {
shader::shader() : vertName("default"), fragName("default") {}
shader::shader(std::string_view vertex_shader, std::string_view fragment_shader)
    : vertName(vertex_shader), fragName(fragment_shader) {}
void shader::set_vertex_shader(std::string_view vertex_shader) {
  vertName = vertex_shader;
}
void shader::set_fragment_shader(std::string_view fragment_shader) {
  fragName = fragment_shader;
}
const std::string &shader::vertex_shader() const { return vertName; }
const std::string &shader::fragment_shader() const { return fragName; }
} // namespace sinen
