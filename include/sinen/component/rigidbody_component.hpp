#ifndef SINEN_RIGIDBODY_COMPONENT_HPP
#define SINEN_RIGIDBODY_COMPONENT_HPP
#include "component.hpp"
namespace sinen {
/**
 * @brief Rigidbody component
 *
 */
class rigidbody_component : public component {
public:
  rigidbody_component(actor &owner);
  virtual ~rigidbody_component();
  virtual void update(float delta_time) override;
  virtual std::string get_name() const override { return "rigidbody"; }
};
} // namespace sinen
#endif // SINEN_RIGIDBODY_COMPONENT_HPP