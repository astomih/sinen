#include <Nen.hpp>
namespace nen
{
	EffectComponent::EffectComponent(Actor &actor, int updateOrder)
		: Component(actor, updateOrder), mOwner(actor), mTimer(0.f), playing(false), isStop(true), mEffect(nullptr)
	{
	}

	EffectComponent::~EffectComponent()
	{
		mOwner.GetScene().GetRenderer()->RemoveEffect(mEffect);
		delete mEffect;
		mEffect = nullptr;
	}

	void EffectComponent::Create(std::u16string_view filePath)
	{
		mEffect = new Effect(filePath);
		mOwner.GetScene().GetRenderer()->AddEffect(mEffect);
	}

	void EffectComponent::Update(float deltaTime)
	{
		if (mEffect)
		{
			auto pos = mOwner.GetPosition();
			mEffect->SetPosition(pos);
		}
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
