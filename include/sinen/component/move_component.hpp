#ifndef SINEN_MOVE_COMPONENT_HPP
#define SINEN_MOVE_COMPONENT_HPP
#include "component.hpp"
namespace sinen {
/**
 * @brief Move component
 *
 */
class move_component : public component {
public:
  move_component(actor &owner);
  virtual void update(float delta_time) override;
  virtual std::string get_name() const override { return "move"; }
};
} // namespace sinen
#endif // SINEN_MOVE_COMPONENT_HPP