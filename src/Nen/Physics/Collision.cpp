#include <Nen.hpp>

namespace nen {
int collision::IntersectAABB(const vector3 &a, const vector3 &b,
                             const vector3 &detectSpace) {
  int i = 0b111;
  if (!(a.x - detectSpace.x <= b.x && b.x <= a.x + detectSpace.x))
    i -= 0b001;
  if (!(a.y - detectSpace.y <= b.y && b.y <= a.y + detectSpace.y))
    i -= 0b010;
  if (!(a.z - detectSpace.z <= b.z && b.z <= a.z + detectSpace.z))
    i -= 0b100;
  return i;
}
} // namespace nen
