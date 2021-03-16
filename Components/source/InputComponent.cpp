#include <Components.hpp>
#include <Actors.hpp>
#include <Engine.hpp>
#include <iostream>

InputComponent::InputComponent(class Actor& owner)
	:MoveComponent(owner)
	, mForwardKey(0)
	, mBackKey(0)
	, mClockwiseKey(0)
	, mCounterClockwiseKey(0)
	, mLeftMoveKey(0)
	, mMaxAngularSpeed(0)
	, mMaxForwardSpeed(0)
	, mMaxLeftRightSpeed(0)
	, mRightMoveKey(0)
{
}

void InputComponent::ProcessInput(const InputState& state)
{
	SetUseDirectionMode(mUseDirectionMode);
	if (mUseDirectionMode)
	{
		// Calculate forward speed for MoveComponent
		float forwardSpeed = 0.0f;
		if (state.Keyboard.GetKeyValue(SDL_Scancode(mForwardKey)))
		{
			forwardSpeed += mMaxForwardSpeed;
		}
		if (state.Keyboard.GetKeyValue(SDL_Scancode(mBackKey)))
		{
			forwardSpeed -= mMaxForwardSpeed;
		}
		SetForwardSpeed(forwardSpeed);

		// Calculate angular speed for MoveComponent
		float angularSpeed = 0.0f;
		if (state.Keyboard.GetKeyValue(SDL_Scancode(mClockwiseKey)))
		{
			angularSpeed += mMaxAngularSpeed;
		}
		if (state.Keyboard.GetKeyValue(SDL_Scancode(mCounterClockwiseKey)))
		{
			angularSpeed -= mMaxAngularSpeed;
		}
		SetAngularSpeed(angularSpeed);
	}
	else
	{
		float forwardSpeed = 0.0f;
		if (state.Keyboard.GetKeyValue(SDL_Scancode(mForwardKey)))
		{
			forwardSpeed += mMaxForwardSpeed;
		}
		if (state.Keyboard.GetKeyValue(SDL_Scancode(mBackKey)))
		{
			forwardSpeed -= mMaxForwardSpeed;
		}
		SetForwardSpeed(forwardSpeed);

		float leftRightSpeed = 0.0f;
		if (state.Keyboard.GetKeyValue(SDL_Scancode(mRightMoveKey)))
		{
			leftRightSpeed += mMaxLeftRightSpeed;
		}
		if (state.Keyboard.GetKeyValue(SDL_Scancode(mLeftMoveKey)))
		{
			leftRightSpeed -= mMaxLeftRightSpeed;
		}
		SetLeftRightSpeed(leftRightSpeed);
	}
}