#pragma once
#include "Component.hpp"
#include "../Audio/SoundEvent.hpp"
#include <vector>
#include <string>

namespace nen
{
	class SoundComponent : public Component
	{
	public:
		SoundComponent(class Actor &owner, int updateOrder = 200);
		~SoundComponent();

		void Update(float deltaTime) override;
		void OnUpdateWorldTransform() override;
		void StopAllEvents();
		void AddEvent(const SoundEvent &e);

	private:
		std::vector<SoundEvent> mEvents;
	};
}