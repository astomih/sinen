#include <Components.hpp>
#include <Actors.hpp>
#include <Scene.hpp>
#include <Engine.hpp>
namespace nen
{
	AudioComponent::AudioComponent(Actor &owner, int updateOrder)
		: Component(owner, updateOrder)
	{
	}

	AudioComponent::~AudioComponent()
	{
		StopAllEvents();
	}

	void AudioComponent::Update(float deltaTime)
	{
		for (auto &i : mEvents)
		{
			i.SetPosition(mOwner.GetPosition());
		}
	}

	void AudioComponent::OnUpdateWorldTransform()
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

	void AudioComponent::StopAllEvents()
	{
		// Stop all sounds
		for (auto &e : mEvents)
		{
			e.Stop();
		}
		// Clear events
		mEvents.clear();
	}

	void AudioComponent::AddEvent(const AudioEvent &e)
	{
		mEvents.emplace_back(e);
	}
}