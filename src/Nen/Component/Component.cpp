#include <Nen.hpp>

namespace nen
{
	Component::Component(Actor &owner, int updateOrder)
		: mOwner(owner), mUpdateOrder(updateOrder)
	{
	}

	Component::~Component()
	{
	}

	void Component::Update(float deltaTime)
	{
	}
	const InputState &Component::GetInput()
	{
		return mOwner.GetInput();
	}
}