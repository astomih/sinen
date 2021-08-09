#include <nen.hpp>
namespace nen
{
    void VertexArray::PushIndices(uint32_t indexArray[], size_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            indices.push_back(indexArray[i]);
        }
    }
}