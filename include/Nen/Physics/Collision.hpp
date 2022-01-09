#include "../Math/Math.hpp"
#include "../Math/Vector3.hpp"

namespace nen {
class collision {
public:
  static bool IntersectAABB(const vector3 &a, const vector3 &b,
                            const vector3 &detectSpace);
};
} // namespace nen