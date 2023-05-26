#ifndef SINEN_SHADER_COMPONENT_HPP
#define SINEN_SHADER_COMPONENT_HPP
#include "../shader/shader.hpp"
#include "component.hpp"

namespace sinen {
/**
 * @brief Shader component
 *
 */
class shader_component : public component {
public:
  shader_component(actor &owner);
  ~shader_component() override;
  void update(float delta_time) override;
  std::string get_name() const override;

  void set_shader(const shader &s) {
    m_shader = s;
    m_loaded = true;
    m_compile = true;
  }

private:
  shader m_shader;
  bool m_loaded = false;
  bool m_compile = false;
};
} // namespace sinen
#endif // SINEN_SHADER_COMPONENT_HPP