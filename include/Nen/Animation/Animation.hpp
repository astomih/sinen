#pragma once
#include "BoneTransform.hpp"
#include "Node.hpp"
#include <vector>
#include <string>

namespace nen
{
    struct VectorKey
    {
        double time;
        Vector3 value;
    };

    struct QuatKey
    {
        double time;
        Quaternion value;
    };
    struct NodeAnim
    {
        std::string node_name;

        std::vector<VectorKey> translate;
        std::vector<VectorKey> scaling;
        std::vector<QuatKey> rotation;
    };
    class Animation
    {
    public:
        float mDuration;
        std::vector<NodeAnim> body;
    };
}