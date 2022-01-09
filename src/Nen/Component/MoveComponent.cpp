#include <Nen.hpp>
#include <iostream>
namespace nen {
move_component::move_component(class base_actor &owner, int updateOrder)
    : base_component(owner, updateOrder), mAngularSpeed(0.0f),
      mForwardSpeed(0.0f) {}

void move_component::Update(float deltaTime) {
  if (!Math::NearZero(axis.x) || !Math::NearZero(axis.y) ||
      !Math::NearZero(axis.z)) {
    mOwner.SetPosition(mOwner.GetPosition() + axis * deltaTime);
  }
  if (!Math::NearZero(mAngularSpeed)) {
    quaternion rot = mOwner.GetRotation();
    float angle = mAngularSpeed * deltaTime;
    // Create quaternion for incremental rotation
    // (Rotate about up axis)
    quaternion inc(vector3::UnitZ, angle);
    // Concatenate old and new quaternion
    rot = quaternion::Concatenate(rot, inc);
    mOwner.SetRotation(rot);
  }

  if (!Math::NearZero(mForwardSpeed)) {
    vector3 pos = mOwner.GetPosition();
    if (mDirection) {
      pos += mOwner.GetForward() * mForwardSpeed * deltaTime;
    } else {
      pos.y += mForwardSpeed * deltaTime;
    }
    mOwner.SetPosition(pos);
  }
  if (!Math::NearZero(mLeftRightSpeed)) {
    vector3 pos = mOwner.GetPosition();
    if (!mDirection) {
      pos.x += mLeftRightSpeed * deltaTime;
    }
    mOwner.SetPosition(pos);
  }
}
} // namespace nen