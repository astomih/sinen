#pragma once
#include "../Input/InputSystem.hpp"
#include <cstdint>
#include <memory>

namespace nen {
class base_actor;
class base_component {
public:
  // Constructor
  // (the lower the update order, the earlier the component updates)
  base_component(class base_actor &owner, int updateOrder = 100);
  // Destructor
  virtual ~base_component();
  // Update this component by delta time
  virtual void Update(float deltaTime);
  // Called when world transform changes
  virtual void OnUpdateWorldTransform() {}

  int GetUpdateOrder() const { return mUpdateOrder; }
  base_actor &GetActor() { return mOwner; }

  void AddedActor() { addedComponentList = true; }

  const input_state &GetInput();

protected:
  // Owning actor
  base_actor &mOwner;
  // Update order of component
  int mUpdateOrder;
  bool addedComponentList = false;
};
} // namespace nen
