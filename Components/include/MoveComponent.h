#pragma once
#include <iostream>

namespace nen
{
	class MoveComponent : public Component
	{
	public:
		// Lower update order to update first
		MoveComponent(class Actor& owner, int updateOrder = 10);
		void Update(float deltaTime) override;

		float GetAngularSpeed() const { return mAngularSpeed; }
		float GetForwardSpeed() const { return mForwardSpeed; }
		float GetLeftRightSpeed() const { return mLeftRightSpeed; }

		void SetAngularSpeed(float speed) {
			mAngularSpeed = speed;
		}
		void SetForwardSpeed(float speed) { mForwardSpeed = speed; }
		void SetLeftRightSpeed(float speed) { mLeftRightSpeed = speed; }
		void SetUseDirectionMode(bool use) { mDirection = use; }
	private:
		bool mDirection = true;
		float mAngularSpeed = 0.f;
		float mForwardSpeed = 0.f;
		float mLeftRightSpeed = 0.f;
	};
}
