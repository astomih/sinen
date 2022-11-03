#ifndef SINEN_PRIMITIVE3_HPP
#define SINEN_PRIMITIVE3_HPP
#include "../math/vector3.hpp"

namespace sinen {
/**
 * @brief Primitive3 class
 *
 */
class primitive3 {
public:
  virtual bool intersects_line3(const struct line3 &l) { return false; }
  virtual bool intersects_segment3(const struct segment3 &s) { return false; }
  virtual bool intersects_sphere(const struct sphere &s) { return false; }
  virtual bool intersects_capsule(const struct capsule &c) { return false; }
  virtual bool intersects_aabb(const struct aabb &a) { return false; }
};
struct line3 : public primitive3 {
  line3();
  line3(const vector3 &p, const vector3 &v);
  vector3 get_point(float t) const { return v * t; }

  vector3 p;
  vector3 v;
};

struct segment3 : public line3 {
  segment3();
  segment3(const vector3 &p, const vector3 &v);
};

struct sphere : public primitive3 {
  sphere() = default;
  ~sphere() = default;
  sphere(const vector3 &p, float r) : p(p), r(r) {}
  vector3 p;
  float r;
};
struct capsule : public primitive3 {
  capsule() : r(0.5f) {}
  capsule(const segment3 &s, float r) : s(s), r(r) {}
  capsule(const vector3 &p1, const vector3 &p2, float r) : s(p1, p2), r(r) {}
  ~capsule() {}
  segment3 s;
  float r;
};

struct aabb : public primitive3 {
  vector3 min;
  vector3 max;
  aabb() = default;
  ~aabb() = default;
  aabb(const vector3 &min, const vector3 &max) : min(min), max(max) {}

  bool intersects_aabb(const aabb &other) override;
};
} // namespace sinen
#endif // !SINEN_PRIMITIVE_HPP
