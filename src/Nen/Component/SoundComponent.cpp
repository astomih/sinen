#include <Nen.hpp>
namespace nen
{
	SoundComponent::SoundComponent(Actor &owner, int updateOrder)
		: Component(owner, updateOrder)
	{
	}

	SoundComponent::~SoundComponent()
	{
		StopAllEvents();
	}

	void SoundComponent::Update(float deltaTime)
	{
		for (auto &i : mEvents)
		{
			i.SetPosition(mOwner.GetPosition());
		}
	}

	void SoundComponent::OnUpdateWorldTransform()
	{
		// Update 3D events' world transforms
		Matrix4 world = mOwner.GetWorldTransform();
		for (auto &event : mEvents)
		{
			if (event.IsValid())
			{
			}
		}
	}

	void SoundComponent::StopAllEvents()
	{
		// Stop all sounds
		for (auto &e : mEvents)
		{
			e.Stop();
		}
		// Clear events
		mEvents.clear();
	}

	void SoundComponent::AddEvent(const SoundEvent &e)
	{
		mEvents.emplace_back(e);
	}
}