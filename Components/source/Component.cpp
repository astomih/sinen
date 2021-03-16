#include <Components.hpp>
#include <Actors.hpp>
#include <Engine.hpp>

Component::Component(Actor &owner, int updateOrder)
	: mOwner(owner), mUpdateOrder(updateOrder)
{
}

Component::~Component()
{
	mOwner.RemoveComponent(shared_from_this());
}

void Component::Update(float deltaTime)
{
}