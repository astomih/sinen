#pragma once
#include <Math/Vector3.hpp>
#include <Math/Vector2.hpp>
#include <array>
namespace nen
{
    struct Vertex
    {
        Vector3 pos;
        std::array<float, 3> norm;
        Vector2 uv;
    };
}