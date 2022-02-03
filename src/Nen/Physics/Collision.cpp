#include <Nen.hpp>

namespace nen {
int collision::IntersectAABB(const vector3 &a, const vector3 &b,
                             const vector3 &detectSpace) {
  int i = 7;
  if (!(a.x - detectSpace.x <= b.x && b.x <= a.x + detectSpace.x)) {
    i -= 1;
    return i;
  }
  if (!(a.y - detectSpace.y <= b.y && b.y <= a.y + detectSpace.y)) {
    i -= 2;
    return i;
  }
  if (!(a.z - detectSpace.z <= b.z && b.z <= a.z + detectSpace.z)) {
    i -= 4;
    return i;
  }
  return i;
}
} // namespace nen
