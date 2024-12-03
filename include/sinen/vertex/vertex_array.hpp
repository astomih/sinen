#ifndef SINEN_VERTEX_ARRAY_HPP
#define SINEN_VERTEX_ARRAY_HPP
// std
#include <cstdint>
#include <vector>

// internal
#include "../vertex/vertex.hpp"

namespace sinen {
/**
 * @brief Vertex array for drawing
 *
 */
struct VertexArray {
public:
  /**
   * @brief Push the ndices of the vertex array
   *
   * @param indiceArray indices
   * @param count count
   */
  void push_indices(std::uint32_t indiceArray[], size_t count);
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  std::string materialName;
  uint32_t indexCount;
};
} // namespace sinen
#endif // !SINEN_VERTEX_ARRAY_HPP
