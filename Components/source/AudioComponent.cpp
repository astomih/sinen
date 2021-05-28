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

		auto iter = mEvents.begin();
		while (iter != mEvents.end())
		{
			iter->SetPosition(mOwner.GetPosition());
			if (!iter->IsValid())
			{
				iter = mEvents.erase(iter);
			}
			else
				++iter;
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
		for (auto &i : mEvents)
		{
			if (i.GetName() == i.GetName())
				return;
		}
		mEvents.push_back(e);
		mEvents[0].SetVolume(100.f);
	}
}