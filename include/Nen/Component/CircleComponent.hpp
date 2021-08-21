#pragma once
#include "Component.hpp"
namespace nen
{
	class CircleComponent : public Component
	{
	public:
		CircleComponent(class Actor &owner);

		void SetRadius(float radius) noexcept { mRadius = radius; }
		const Vector3 &GetRadius() const;

		const Vector3 &GetCenter() const;

	private:
		float mRadius;
	};

	bool Intersect(const CircleComponent &a, const CircleComponent &b);
}
