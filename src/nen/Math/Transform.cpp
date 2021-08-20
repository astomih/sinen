#include <nen.hpp>
namespace nen
{
	void Transform::LookAt(const Vector3& target,const Vector3& norm)
	{
		Vector3  vec = target - mOwner.GetPosition();
		vec.Normalize();
		float dot = Vector3::Dot(norm, vec);
		float theta = Math::Acos(dot);
		Vector3 cross = Vector3::Cross(norm, vec);
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
}
