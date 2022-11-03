#ifndef SINEN_VERTEX_ARRAY_HPP
#define SINEN_VERTEX_ARRAY_HPP
#include "../vertex/vertex.hpp"
#include <vector>

namespace sinen {
/**
 * @brief Vertex array for drawing
 *
 */
struct vertex_array {
public:
  /**
   * @brief Push the ndices of the vertex array
   *
   * @param indiceArray indices
   * @param count count
   */
  void push_indices(uint32_t indiceArray[], size_t count);
  std::vector<vertex> vertices;
  std::vector<uint32_t> indices;
  std::string materialName;
  uint32_t indexCount;
};
} // namespace sinen
#endif // !SINEN_VERTEX_ARRAY_HPP
