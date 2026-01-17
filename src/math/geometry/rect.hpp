#ifndef SINEN_RECT_HPP
#define SINEN_RECT_HPP
#include "mesh.hpp"
#include "pivot.hpp"
#include <core/data/ptr.hpp>
#include <math/vector.hpp>
namespace sinen {

struct Rect {
  Rect();
  Rect(float x, float y, float width, float height);
  Rect(const Vec2 &pos, const Vec2 &size);
  Rect(Pivot pivot, float x, float y, float width, float height);
  Rect(Pivot pivot, const Vec2 &pos, const Vec2 &size);
  union {
    struct {
      float x;
      float y;
      float width;
      float height;
    };
    struct {
      Vec2 _pos;
      Vec2 _size;
    };
  };

  Vec2 topLeft() const;
  Vec2 topCenter() const;
  Vec2 topRight() const;
  Vec2 center() const;
  Vec2 bottomLeft() const;
  Vec2 bottomCenter() const;
  Vec2 bottomRight() const;

  Vec2 position() const { return _pos; }
  Vec2 positionfromPivot(Pivot pivot) const;
  Vec2 size() const { return _size; }

  bool intersectsRect(const Rect &rect);
  Mesh createMesh();
};
} // namespace sinen
#endif
