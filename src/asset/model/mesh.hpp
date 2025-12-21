#ifndef SINEN_VERTEX_ARRAY_HPP
#define SINEN_VERTEX_ARRAY_HPP
// internal
#include "vertex.hpp"
#include <core/data/array.hpp>
#include <core/data/string.hpp>
#include <core/def/types.hpp>

namespace sinen {
/**
 * @brief Vertex array for drawing
 *
 */
struct Mesh {
public:
  /**
   * @brief Push the ndices of the vertex array
   *
   * @param indiceArray indices
   * @param count count
   */
  void pushIndices(uint32 indiceArray[], size_t count);
  Array<Vertex> vertices;
  Array<glm::vec4> tangents;
  Array<uint32> indices;
  String materialName;
  uint32 indexCount;
};
} // namespace sinen
#endif // !SINEN_VERTEX_ARRAY_HPP
