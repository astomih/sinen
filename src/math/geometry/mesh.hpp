#ifndef SINEN_VERTEX_ARRAY_HPP
#define SINEN_VERTEX_ARRAY_HPP
// internal
#include "vertex.hpp"
#include <core/data/array.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <core/def/types.hpp>

namespace sinen {
/**
 * @brief Vertex array for drawing
 *
 */
class Mesh {
public:
  struct Data {
    struct Lod {
      Array<UInt32> indices;
      float threshold = 0.0f;
      float error = 0.0f;
    };
    Array<Vertex> vertices;
    Array<Vec4> tangents;
    Array<UInt32> indices;
    Array<Lod> lods;
  };
  Mesh();
  explicit Mesh(const Data &data);
  explicit Mesh(const Ptr<Data> &data);

  Ptr<Data> data() const { return _data; }

private:
  Ptr<Data> _data;
};
} // namespace sinen
#endif // !SINEN_VERTEX_ARRAY_HPP
