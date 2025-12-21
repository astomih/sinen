#include <asset/model/mesh.hpp>
namespace sinen {
void Mesh::pushIndices(uint32 indexArray[], size_t count) {
  for (size_t i = 0; i < count; i++) {
    indices.push_back(indexArray[i]);
  }
}
} // namespace sinen
