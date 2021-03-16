#include <Engine.hpp>
#include <Components.hpp>
#include <Scene.hpp>
#include <Actors.hpp>

//TODO
EffectComponent::EffectComponent(Actor &actor)
	: Component(actor)
{
}

EffectComponent::~EffectComponent()
{
}

void EffectComponent::Draw()
{
}

void EffectComponent::Update(float deltaTime)
{
}

void EffectComponent::Play(int32_t startFrame)
{
}
