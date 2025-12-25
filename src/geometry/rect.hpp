#ifndef SINEN_RECT_HPP
#define SINEN_RECT_HPP
#include "mesh.hpp"
#include <core/data/ptr.hpp>
#include <math/vector.hpp>
namespace sinen {

struct Rect {
  Rect() = default;
  Rect(float x, float y, float width, float height)
      : x(x), y(y), width(width), height(height) {}
  Rect(const Vec2 &p, const Vec2 &s) : p(p), s(s) {}
  union {
    struct {
      float x;
      float y;
      float width;
      float height;
    };
    struct {
      Vec2 p;
      Vec2 s;
    };
  };
  Ptr<Mesh> createMesh();
};
} // namespace sinen
#endif