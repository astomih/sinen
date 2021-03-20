#pragma once
#include <Engine/include/Vertex.h>

class VertexArray
{
public:
    void PushIndices(uint32_t indiceArray[], size_t count);
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};