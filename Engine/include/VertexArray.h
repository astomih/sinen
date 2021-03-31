#pragma once
#include <Engine/include/Vertex.h>

namespace nen
{
    class VertexArray
    {
    public:
        void PushIndices(uint32_t indiceArray[], size_t count);
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::string materialName;
        uint32_t indexCount;
    };
}