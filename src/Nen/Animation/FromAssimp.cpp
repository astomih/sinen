#include "FromAssimp.hpp"

namespace nen
{
    Vector3 fromAssimp(const aiVector3D &v)
    {
        return Vector3{v.x, v.y, v.z};
    }

    Color fromAssimp(const aiColor3D &col)
    {
        return Color{col.r, col.g, col.b, 1.0};
    }

    Color fromAssimp(const aiColor4D &col)
    {
        return Color{col.r, col.g, col.b, col.a};
    }

    VectorKey fromAssimp(const aiVectorKey &key)
    {
        VectorKey v;

        v.time = key.mTime;
        v.value = fromAssimp(key.mValue);

        return v;
    }

    QuatKey fromAssimp(const aiQuatKey &key)
    {
        QuatKey v;

        v.time = key.mTime;
        v.value = Quaternion{key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w};

        return v;
    }
}