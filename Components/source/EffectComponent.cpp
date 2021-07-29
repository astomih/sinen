#include <Engine.hpp>
#include <Components.hpp>
#include <Scene.hpp>
#include <Actors.hpp>
namespace nen
{
	//TODO
	EffectComponent::EffectComponent(Actor &actor, std::u16string_view filepath)
		: Component(actor), mOwner(actor), path(filepath), mTimer(0.f), playing(false), isStop(true)
	{
		mOwner.GetScene().GetRenderer()->AddEffectComp(this);
	}

	EffectComponent::~EffectComponent()
	{
		mOwner.GetScene().GetRenderer()->RemoveEffectComp(this);
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
