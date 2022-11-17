#ifndef SINEN_DRAW_COMPONENT_HPP
#define SINEN_DRAW_COMPONENT_HPP
#include "component.hpp"
namespace sinen {
/**
 * @brief TODO: draw_component
 *
 */
class draw_component : public component {
public:
  /**
   * @brief Construct a new draw component object
   *
   * @param owner The actor that owns this component
   */
  draw_component(actor &owner);
  /**
   * @brief Destroy the draw component object
   *
   */
  virtual ~draw_component();
  /**
   * @brief Update the component
   *
   * @param delta_time delta time
   */
  virtual void update(float delta_time) override;
};
} // namespace sinen
#endif // !SINEN_DRAW_COMPONENT_HPP