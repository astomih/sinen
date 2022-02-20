#include <Nen.hpp>
#include <algorithm>
#include <cstdint>
#include <type_traits>
namespace nen {
base_actor::base_actor(base_scene &scene)
    : mState(state::Active), mPosition(vector3::Zero),
      mRotation(quaternion::Identity), mScene(scene),
      mRecomputeWorldTransform(true), mScale(vector3(1.f, 1.f, 1.f)) {}

base_actor::~base_actor() { m_components.clear(); }

void base_actor::UpdateActor(float deltaTime) {
  if (mState == state::Active) {
    for (const auto &comp : m_components.data) {
      comp.second->Update(deltaTime);
    }
    Update(deltaTime);
    ComputeWorldTransform();
  }
}

void base_actor::Update(float deltaTime) {}

void base_actor::ComputeWorldTransform() {
  if (mRecomputeWorldTransform) {
    this->RecomuteFinished();

    auto scaleOwner = GetScale();
    auto pos = GetPosition();
    matrix4 t = matrix4::Identity;
    t.mat[3][0] = pos.x;
    t.mat[3][1] = pos.y;
    t.mat[3][2] = pos.z;
    matrix4 r = matrix4::CreateFromQuaternion(this->GetRotation());
    matrix4 s = matrix4::Identity;
    s.mat[0][0] = scaleOwner.x;
    s.mat[1][1] = scaleOwner.y;
    s.mat[2][2] = scaleOwner.z;
    this->mWorldTransform = s * r * t;

    // Inform components world transform updated
    for (const auto &comp : m_components.data) {
      comp.second->OnUpdateWorldTransform();
    }
  }
}
const input_state &base_actor::GetInput() { return mScene.GetInput(); }

} // namespace nen
