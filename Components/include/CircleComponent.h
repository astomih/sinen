#pragma once
#include <Engine.hpp>
namespace nen
{
	class CircleComponent : public Component
	{
	public:
		CircleComponent(class Actor &owner);

		void SetRadius(float radius) noexcept { mRadius = radius; }
		const Vector3f &GetRadius() const;

		const Vector3f &GetCenter() const;

	private:
		float mRadius;
	};

	bool Intersect(const CircleComponent &a, const CircleComponent &b);
}
