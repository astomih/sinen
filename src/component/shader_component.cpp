#include <component/shader_component.hpp>
#include <render/renderer.hpp>
#include <shader/shader.hpp>

namespace sinen {
shader_component::shader_component(Actor &owner) : component(owner) {}
shader_component::~shader_component() {}
void shader_component::update(float delta_time) {
  if (!m_loaded) {
    return;
  }
  if (m_compile) {
    m_compile = false;
    m_shader.load();
  }
}
std::string shader_component::get_name() const { return "shader"; }
} // namespace sinen