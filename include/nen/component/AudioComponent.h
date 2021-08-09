#pragma once
#include "../nen.hpp"
#include <vector>
#include <string>

namespace nen
{
	class AudioComponent : public Component
	{
	public:
		AudioComponent(class Actor &owner, int updateOrder = 200);
		~AudioComponent();

		void Update(float deltaTime) override;
		void OnUpdateWorldTransform() override;
		void StopAllEvents();
		void AddEvent(const AudioEvent &e);

	private:
		std::vector<AudioEvent> mEvents;
	};
}