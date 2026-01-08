#include "vec4.hpp"

namespace sinen {
Vec4::Vec4(const Vec3 &v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
Vec4::Vec4(float x, const Vec3 &v) : x(x), y(v.x), z(v.y), w(v.z) {}
} // namespace sinen