#include <Nen.hpp>
#include <iostream>
namespace nen {
input_component::input_component(class base_actor &owner)
    : move_component(owner), mForwardKey(0), mBackKey(0), mClockwiseKey(0),
      mCounterClockwiseKey(0), mLeftMoveKey(0), mMaxAngularSpeed(0),
      mMaxForwardSpeed(0), mMaxLeftRightSpeed(0), mRightMoveKey(0) {}

void input_component::Update(float deltaTime) {
  auto state = GetInput();
  SetUseDirectionMode(mUseDirectionMode);
  if (mUseDirectionMode) {
    // Calculate forward speed for MoveComponent
    float forwardSpeed = 0.0f;
    if (state.Keyboard.GetKeyValue(static_cast<key_code>(mForwardKey))) {
      forwardSpeed += mMaxForwardSpeed;
    }
    if (state.Keyboard.GetKeyValue(static_cast<key_code>(mBackKey))) {
      forwardSpeed -= mMaxForwardSpeed;
    }
    SetForwardSpeed(forwardSpeed);

    // Calculate angular speed for MoveComponent
    float angularSpeed = 0.0f;
    if (state.Keyboard.GetKeyValue(static_cast<key_code>(mClockwiseKey))) {
      angularSpeed += mMaxAngularSpeed;
    }
    if (state.Keyboard.GetKeyValue(
            static_cast<key_code>(mCounterClockwiseKey))) {
      angularSpeed -= mMaxAngularSpeed;
    }
    SetAngularSpeed(angularSpeed);
  } else {
    float forwardSpeed = 0.0f;
    if (state.Keyboard.GetKeyValue(static_cast<key_code>(mForwardKey))) {
      forwardSpeed += mMaxForwardSpeed;
    }
    if (state.Keyboard.GetKeyValue(static_cast<key_code>(mBackKey))) {
      forwardSpeed -= mMaxForwardSpeed;
    }
    SetForwardSpeed(forwardSpeed);

    float leftRightSpeed = 0.0f;
    if (state.Keyboard.GetKeyValue(static_cast<key_code>(mRightMoveKey))) {
      leftRightSpeed += mMaxLeftRightSpeed;
    }
    if (state.Keyboard.GetKeyValue(static_cast<key_code>(mLeftMoveKey))) {
      leftRightSpeed -= mMaxLeftRightSpeed;
    }
    SetLeftRightSpeed(leftRightSpeed);
  }
}
} // namespace nen