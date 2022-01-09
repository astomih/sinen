#pragma once
#include "../Math/Vector3.hpp"
#include "Component.hpp"
#include <iostream>


namespace nen {
class move_component : public base_component {
public:
  // Lower update order to update first
  move_component(class base_actor &owner, int updateOrder = 10);
  void Update(float deltaTime) override;

  float GetAngularSpeed() const { return mAngularSpeed; }
  float GetForwardSpeed() const { return mForwardSpeed; }
  float GetLeftRightSpeed() const { return mLeftRightSpeed; }
  vector3 GetAxisSpeed() const { return axis; }

  void SetAngularSpeed(float speed) { mAngularSpeed = speed; }
  void SetForwardSpeed(float speed) { mForwardSpeed = speed; }
  void SetLeftRightSpeed(float speed) { mLeftRightSpeed = speed; }
  void SetUseDirectionMode(bool use) { mDirection = use; }
  void SetAxisSpeed(const vector3 &axis) { this->axis = axis; }

private:
  bool mDirection = true;
  float mAngularSpeed = 0.f;
  float mForwardSpeed = 0.f;
  float mLeftRightSpeed = 0.f;
  vector3 axis;
};
} // namespace nen
