#include <Nen.hpp>
#include <iostream>
namespace nen
{
	InputComponent::InputComponent(class Actor &owner)
		: MoveComponent(owner), mForwardKey(0), mBackKey(0), mClockwiseKey(0), mCounterClockwiseKey(0), mLeftMoveKey(0), mMaxAngularSpeed(0), mMaxForwardSpeed(0), mMaxLeftRightSpeed(0), mRightMoveKey(0)
	{
	}

	void InputComponent::Update(float deltaTime)
	{
		auto state = GetInput();
		SetUseDirectionMode(mUseDirectionMode);
		if (mUseDirectionMode)
		{
			// Calculate forward speed for MoveComponent
			float forwardSpeed = 0.0f;
			if (state.Keyboard.GetKeyValue(static_cast<KeyCode>(mForwardKey)))
			{
				forwardSpeed += mMaxForwardSpeed;
			}
			if (state.Keyboard.GetKeyValue(static_cast<KeyCode>(mBackKey)))
			{
				forwardSpeed -= mMaxForwardSpeed;
			}
			SetForwardSpeed(forwardSpeed);

			// Calculate angular speed for MoveComponent
			float angularSpeed = 0.0f;
			if (state.Keyboard.GetKeyValue(static_cast<KeyCode>(mClockwiseKey)))
			{
				angularSpeed += mMaxAngularSpeed;
			}
			if (state.Keyboard.GetKeyValue(static_cast<KeyCode>(mCounterClockwiseKey)))
			{
				angularSpeed -= mMaxAngularSpeed;
			}
			SetAngularSpeed(angularSpeed);
		}
		else
		{
			float forwardSpeed = 0.0f;
			if (state.Keyboard.GetKeyValue(static_cast<KeyCode>(mForwardKey)))
			{
				forwardSpeed += mMaxForwardSpeed;
			}
			if (state.Keyboard.GetKeyValue(static_cast<KeyCode>(mBackKey)))
			{
				forwardSpeed -= mMaxForwardSpeed;
			}
			SetForwardSpeed(forwardSpeed);

			float leftRightSpeed = 0.0f;
			if (state.Keyboard.GetKeyValue(static_cast<KeyCode>(mRightMoveKey)))
			{
				leftRightSpeed += mMaxLeftRightSpeed;
			}
			if (state.Keyboard.GetKeyValue(static_cast<KeyCode>(mLeftMoveKey)))
			{
				leftRightSpeed -= mMaxLeftRightSpeed;
			}
			SetLeftRightSpeed(leftRightSpeed);
		}
	}
}