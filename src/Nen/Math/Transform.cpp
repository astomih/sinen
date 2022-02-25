#include <Nen.hpp>
namespace nen {
/*
void transform::LookAt(const vector3 &target, const vector3 &norm) {
vector3 vec = target - mOwner.GetPosition();
vec.Normalize();
float dot = vector3::Dot(norm, vec);
float theta = Math::Acos(dot);
vector3 cross = vector3::Cross(norm, vec);
cross.Normalize();
auto q = mOwner.GetRotation();
theta = theta / 2.f;
q.x = Math::Sin(theta) * cross.x;
q.y = Math::Sin(theta) * cross.y;
q.z = Math::Sin(theta) * cross.z;
q.w = Math::Cos(theta);
q.Normalize();
mOwner.SetRotation(q);
}
*/
} // namespace nen
