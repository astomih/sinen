#include <Nen.hpp>

namespace nen {
billboard_component::billboard_component(base_actor &owner, int updateOwner)
    : base_component(owner, updateOwner) {}

void billboard_component::Update(float deltaTime) {
  auto camera = mOwner.GetScene().GetRenderer().GetViewMatrix();
  mOwner.SetRotation(matrix4::ToQuaternion(camera));
}
} // namespace nen