#include <asset/model/vertex_array.hpp>
namespace sinen {
void VertexArray::PushIndices(std::uint32_t indexArray[], size_t count) {
  for (size_t i = 0; i < count; i++) {
    indices.push_back(indexArray[i]);
  }
}
} // namespace sinen
