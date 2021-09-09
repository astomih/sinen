#include <Nen.hpp>

namespace nen
{
    BillboardComponent::BillboardComponent(Actor &owner, int updateOwner)
        : Component(owner, updateOwner)
    {
    }

    void BillboardComponent::Update(float deltaTime)
    {
        auto camera = mOwner.GetScene().GetRenderer()->GetViewMatrix();
        mOwner.SetRotation(Matrix4::ToQuaternion(camera));
    }
}