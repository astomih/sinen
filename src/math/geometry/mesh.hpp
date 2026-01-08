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
  Array<Vertex> vertices;
  Array<Vec4> tangents;
  Array<UInt32> indices;
};
} // namespace sinen
#endif // !SINEN_VERTEX_ARRAY_HPP
