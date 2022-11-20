#ifndef SINEN_DRAW_COMPONENT_HPP
#define SINEN_DRAW_COMPONENT_HPP
#include "../drawable/drawable.hpp"
#include "component.hpp"

namespace sinen {
/**
 * @brief TODO: draw3d_component
 *
 */
class draw3d_component : public component {
public:
  /**
   * @brief Construct a new draw component object
   *
   * @param owner The actor that owns this component
   */
  draw3d_component(actor &owner);
  /**
   * @brief Destroy the draw component object
   *
   */
  virtual ~draw3d_component();
  /**
   * @brief Update the component
   *
   * @param delta_time delta time
   */
  virtual void update(float delta_time) override;

  void set_texture(const texture &_texture) { texture_handle = _texture; }
  void set_draw_depth(bool _is_draw_depth) { is_draw_depth = _is_draw_depth; }
  void set_vertex_name(const std::string &_vertex_name) {
    vertex_name = _vertex_name;
  }

private:
  texture texture_handle;
  bool is_draw_depth = true;
  std::string vertex_name = "SPRITE";
};
} // namespace sinen
#endif // !SINEN_DRAW_COMPONENT_HPP