#pragma once
#include "../Math/Matrix4.hpp"
#include "Weight.hpp"
namespace nen
{
    struct Bone
    {
        std::string name;
        Matrix4 offset;

        std::vector<Weight> weights;
    };
}
