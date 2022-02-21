#include "Component/rigidbody_component.hpp"
#include <Nen.hpp>

namespace nen {
rigidbody_component::rigidbody_component(base_actor &owner)
    : base_component(owner) {}
void rigidbody_component::Update(float dt) {
  aabb buf;
  buf.max = mOwner.GetPosition() +
            mOwner.GetScale() *
                nen::vector3::Transform(m_local_aabb.max, mOwner.GetRotation());
  buf.min = mOwner.GetPosition() +
            mOwner.GetScale() *
                nen::vector3::Transform(m_local_aabb.min, mOwner.GetRotation());
  m_aabb.min.x = std::min(buf.min.x, buf.max.x);
  m_aabb.min.y = std::min(buf.min.y, buf.max.y);
  m_aabb.min.z = std::min(buf.min.z, buf.max.z);
  m_aabb.max.x = std::max(buf.min.x, buf.max.x);
  m_aabb.max.y = std::max(buf.min.y, buf.max.y);
  m_aabb.max.z = std::max(buf.min.z, buf.max.z);
}

bool rigidbody_component::intersects(const rigidbody_component &other) {
  return collision::aabb_aabb(m_aabb, other.get_world_aabb());
}
} // namespace nen