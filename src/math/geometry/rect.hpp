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
      Vec2 pos;
      Vec2 size;
    };
  };

  Rect topLeft() const;
  Rect topCenter() const;
  Rect topRight() const;
  Rect center() const;
  Rect bottomLeft() const;
  Rect bottomCenter() const;
  Rect bottomRight() const;
  Rect fromPivot(Pivot pivot) const;

  bool intersectsRect(const Rect &rect);
  Mesh createMesh();
};
} // namespace sinen
#endif
