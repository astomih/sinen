#pragma once
#include <cstdint>
#include <memory>

namespace nen
{
	class Actor;
	class Component : public std::enable_shared_from_this<Component>
	{
	public:
		// Constructor
		// (the lower the update order, the earlier the component updates)
		Component(class Actor &owner, int updateOrder = 100);
		// Destructor
		virtual ~Component();
		// Update this component by delta time
		virtual void Update(float deltaTime);
		// Process input for this component
		virtual void ProcessInput(const struct InputState &state) {}
		// Called when world transform changes
		virtual void OnUpdateWorldTransform() {}

		int GetUpdateOrder() const { return mUpdateOrder; }
		Actor &GetActor() { return mOwner; }

		void AddedActor() { addedComponentList = true; }

	protected:
		// Owning actor
		Actor &mOwner;
		// Update order of component
		int mUpdateOrder;
		bool addedComponentList = false;

	};
}
