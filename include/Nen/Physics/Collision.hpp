#include "../Math/Vector3.hpp"
#include "../Math/Math.hpp"

namespace nen
{
    class Collision
    {
    public:
        static bool IntersectAABB(const Vector3 &a, const Vector3 &b, const Vector3 &detectSpace);
    };
}