#include <Nen.hpp>
namespace nen
{
	EffectComponent::EffectComponent(Actor &actor, int updateOrder)
		: Component(actor, updateOrder), mOwner(actor)
	{
	}

	EffectComponent::~EffectComponent()
	{
		mOwner.GetScene().GetRenderer()->RemoveEffect(mEffect);
		mEffect = nullptr;
	}

	void EffectComponent::Create(std::string_view filePath)
	{
		mEffect = std::make_shared<Effect>(filePath);
		mOwner.GetScene().GetRenderer()->AddEffect(mEffect);
	}
	void EffectComponent::SetLoop(bool loop, Timer interval)
	{
		mEffect->SetLoop(loop, interval);
	}
	bool EffectComponent::isLoop()
	{
		return mEffect->isLoop();
	}

	void EffectComponent::Update(float deltaTime)
	{
		if (mEffect)
		{
			auto pos = mOwner.GetPosition();
			mEffect->SetPosition(pos);

			mEffect->SetRotation(mOwner.GetRotation());
		}
	}
}
