#pragma once
#include "../Math/Vector3.hpp"
#include "../Math/Quaternion.hpp"
namespace nen
{
    class BoneTransform
    {
    public:
        // For now, just make this data public
        Quaternion mRotation;
        Vector3 mTranslation;

        Matrix4 ToMatrix() const;

        static BoneTransform Interpolate(const BoneTransform &a, const BoneTransform &b, float f);
    };
}