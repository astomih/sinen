#ifndef NEN_RIGIDBODY_COMPONENT_HPP
#define NEN_RIGIDBODY_COMPONENT_HPP
#include "../Physics/Collision.hpp"
#include "Component.hpp"
#include "Physics/primitive.hpp"

namespace nen {
class rigidbody_component : public base_component {
public:
  rigidbody_component(class base_actor &owner);
  ~rigidbody_component() = default;
  void Update(float dt) override;
  void set_local_aabb(const aabb &aabb) { m_local_aabb = aabb; }
  aabb get_local_aabb() const { return m_local_aabb; }
  aabb get_world_aabb() const { return m_aabb; }
  bool intersects(const rigidbody_component &other);

private:
  aabb m_local_aabb;
  aabb m_aabb;
};
} // namespace nen
#endif