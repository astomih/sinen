#include <Nen.hpp>
namespace nen {

circle_component::circle_component(class base_actor &owner)
    : base_component(owner), mRadius(0.0f) {}

const vector3 &circle_component::GetCenter() const {
  return mOwner.GetPosition();
}

vector3 circle_component::GetRadius() const {
  return mOwner.GetScale() * mRadius;
}

bool circle_component::Intersect(const circle_component &a,
                                 const circle_component &b) {
  vector3 diff = a.GetCenter() - b.GetCenter();
  float distSq = diff.LengthSq();

  auto radiiSq = a.GetRadius() + b.GetRadius();
  radiiSq = radiiSq * radiiSq;

  return distSq <= radiiSq.x && distSq <= radiiSq.y;
}
} // namespace nen