#pragma once
#include "Component.hpp"
namespace nen {
class circle_component : public base_component {
public:
  circle_component(class base_actor &owner);

  void SetRadius(float radius) noexcept { mRadius = radius; }
  vector3 GetRadius() const;
  const vector3 &GetCenter() const;
  bool Intersect(const circle_component &a, const circle_component &b);

private:
  float mRadius;
};

} // namespace nen