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
  /**
   * @brief Get the name object
   *
   * @return std::string
   */
  virtual std::string get_name() const override { return "draw3d"; }
  /**
   * @brief set_texture Use texture
   * @param _texture texture instance
   */
  void set_texture(const texture &_texture) { texture_handle = _texture; }
  /**
   * @brief set_vertex_name
   * @param _vertex_name
   */
  void set_vertex_name(std::string_view _vertex_name) {
    vertex_name = _vertex_name;
  }

private:
  std::shared_ptr<drawable> draw_object;
  texture texture_handle;
  std::string vertex_name = "SPRITE";
};
} // namespace sinen
#endif // !SINEN_DRAW_COMPONENT_HPP
