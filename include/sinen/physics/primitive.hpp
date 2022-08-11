#ifndef SINEN_PRIMITIVE3_HPP
#define SINEN_PRIMITIVE3_HPP
#include "../math/vector3.hpp"

namespace sinen {
class primitive {
public:
};
struct line : public primitive {
  line();
  line(const vector3 &p, const vector3 &v);
  vector3 get_point(float t) const { return v * t; }

  vector3 p;
  vector3 v;
};

struct segment : public line {
  segment();
  segment(const vector3 &p, const vector3 &v);
};

struct sphere : public primitive {
  sphere() = default;
  ~sphere() = default;
  sphere(const vector3 &p, float r) : p(p), r(r) {}
  vector3 p;
  float r;
};
struct capsule : public primitive {
  capsule() : r(0.5f) {}
  capsule(const segment &s, float r) : s(s), r(r) {}
  capsule(const vector3 &p1, const vector3 &p2, float r) : s(p1, p2), r(r) {}
  ~capsule() {}
  segment s;
  float r;
};

struct aabb : public primitive {
  vector3 min;
  vector3 max;
  aabb() = default;
  ~aabb() = default;
  aabb(const vector3 &min, const vector3 &max) : min(min), max(max) {}

  bool intersects_aabb(const aabb &other);
};
} // namespace sinen
#endif // !SINEN_PRIMITIVE_HPP
