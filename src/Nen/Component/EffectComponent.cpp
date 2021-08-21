#include <Nen.hpp>
namespace nen
{
	//TODO
	EffectComponent::EffectComponent(Actor &actor, std::u16string_view filepath)
		: Component(actor), mOwner(actor), mTimer(0.f), playing(false), isStop(true)
	{
		mEffect = new Effect(filepath);
		mOwner.GetScene().GetRenderer()->AddEffect(mEffect);
	}

	EffectComponent::~EffectComponent()
	{
		mOwner.GetScene().GetRenderer()->RemoveEffect(mEffect);
		delete mEffect;
		mEffect = nullptr;
	}

	void EffectComponent::Update(float deltaTime)
	{
		if (mTimer != 0.f)
		{
			mTimer -= deltaTime;
			if (mTimer <= 0.f)
			{
				this->playing = !playing;
				mTimer = buf;
			}
		}
	}
}
