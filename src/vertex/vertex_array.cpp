#include <vertex/vertex_array.hpp>
namespace sinen {
void vertex_array::push_indices(uint32_t indexArray[], size_t count) {
  for (size_t i = 0; i < count; i++) {
    indices.push_back(indexArray[i]);
  }
}
} // namespace sinen
