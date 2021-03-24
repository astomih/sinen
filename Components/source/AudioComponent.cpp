#include <Components.hpp>
#include <Actors.hpp>
#include <Scene.hpp>
#include <Engine.hpp>
namespace nen
{
	AudioComponent::AudioComponent(Actor& owner, int updateOrder)
		: Component(owner, updateOrder)
	{
	}

	AudioComponent::~AudioComponent()
	{
		StopAllEvents();
	}

	void AudioComponent::Update(float deltaTime)
	{
		Component::Update(deltaTime);

		// Remove invalid 2D events
		auto iter = mEvents2D.begin();
		while (iter != mEvents2D.end())
		{
			if (!iter->IsValid())
			{
				iter = mEvents2D.erase(iter);
			}
			else
			{
				++iter;
			}
		}

		// Remove invalid 3D events
		iter = mEvents3D.begin();
		while (iter != mEvents3D.end())
		{
			if (!iter->IsValid())
			{
				iter = mEvents3D.erase(iter);
			}
			else
			{
				++iter;
			}
		}
	}

	void AudioComponent::OnUpdateWorldTransform()
	{
		// Update 3D events' world transforms
		Matrix4 world = mOwner.GetWorldTransform();
		for (auto& event : mEvents3D)
		{
			if (event.IsValid())
			{
			}
		}
	}

	void AudioComponent::StopAllEvents()
	{
		// Stop all sounds
		for (auto& e : mEvents2D)
		{
			e.Stop();
		}
		for (auto& e : mEvents3D)
		{
			e.Stop();
		}
		// Clear events
		mEvents2D.clear();
		mEvents3D.clear();
	}
}