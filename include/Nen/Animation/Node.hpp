#pragma once
#include "../Math/Matrix4.hpp"
#include "../Mesh/Mesh.hpp"

namespace nen
{
    class Node
    {
    public:
        std::string name;

        std::vector<Mesh> mesh;

        Matrix4 matrix;
        Matrix4 matrix_orig;
        Matrix4 global_matrix;
        Matrix4 invert_matrix;

        std::vector<std::shared_ptr<Node>> children;
    };
}