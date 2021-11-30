#pragma once
#include "../Math/Matrix4.hpp"

namespace nen
{
    const size_t MAX_SKELETON_BONES = 96;

    struct MatrixPalette
    {
        Matrix4 mEntry[MAX_SKELETON_BONES];
    };
}