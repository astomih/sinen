#pragma once
#include "../Render/Renderer.hpp"
#include "../Vertex/VertexArray.hpp"
#include <memory>
#include <string_view>
#include "Bone.hpp"
namespace nen
{
    class Mesh
    {
    public:
        VertexArray original;
        VertexArray body;

        bool has_bone;
        std::vector<Bone> bones;
    };
}