#include <Nen.hpp>
namespace nen
{

	CircleComponent::CircleComponent(class Actor &owner)
		: Component(owner), mRadius(0.0f)
	{
	}

	const Vector3 &CircleComponent::GetCenter() const
	{
		return mOwner.GetPosition();
	}

	const Vector3 &CircleComponent::GetRadius() const
	{
		return mOwner.GetScale() * mRadius;
	}

	bool Intersect(const CircleComponent &a, const CircleComponent &b)
	{
		Vector3 diff = a.GetCenter() - b.GetCenter();
		float distSq = diff.LengthSq();

		auto radiiSq = a.GetRadius() + b.GetRadius();
		radiiSq = radiiSq * radiiSq;

		return distSq <= radiiSq.x && distSq <= radiiSq.y;
	}
}