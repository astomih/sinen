#include "vertex.hpp"
namespace sinen {

Vertex::Vertex()
    : position(Vec3(0)), normal(Vec3(1)), uv(Vec2(0)), color(Color(1)) {}
Vertex::Vertex(const Vec3 &position, const Vec3 &normal, const Vec2 &uv,
               const Color &color)
    : position(position), normal(normal), uv(uv), color(color) {}
} // namespace sinen