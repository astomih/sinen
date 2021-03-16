#include <Components.hpp>
#include <Actors.hpp>
#include <Engine.hpp>
#include <Scene.hpp>
#include <iostream>

MoveComponent::MoveComponent(class Actor &owner, int updateOrder)
	: Component(owner, updateOrder), mAngularSpeed(0.0f), mForwardSpeed(0.0f)
{
}

void MoveComponent::Update(float deltaTime)
{
	if (!Math::NearZero(mAngularSpeed))
	{
		Quaternion rot = mOwner.GetRotation();
		float angle = mAngularSpeed * deltaTime;
		// Create quaternion for incremental rotation
		// (Rotate about up axis)
		Quaternion inc(Vector3f::UnitZ, angle);
		// Concatenate old and new quaternion
		rot = Quaternion::Concatenate(rot, inc);
		mOwner.SetRotation(rot);
	}

	if (!Math::NearZero(mForwardSpeed))
	{
		Vector3f pos = mOwner.GetPosition();
		if (mDirection)
		{
			pos += mOwner.GetForward() * mForwardSpeed * deltaTime;
		}
		else
		{
			pos.y += mForwardSpeed * deltaTime;
		}
		mOwner.SetPosition(pos);
	}
	if (!Math::NearZero(mLeftRightSpeed))
	{
		Vector3f pos = mOwner.GetPosition();
		if (!mDirection)
		{
			pos.x += mLeftRightSpeed * deltaTime;
		}
		mOwner.SetPosition(pos);
	}
}