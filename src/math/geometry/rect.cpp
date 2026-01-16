#include "rect.hpp"
namespace sinen {
Rect::Rect() : pos(), size() {}

Rect::Rect(float x, float y, float width, float height)
    : x(x), y(y), width(width), height(height) {}

Rect::Rect(const Vec2 &pos, const Vec2 &size) : pos(pos), size(size) {}

Rect::Rect(Pivot pivot, float x, float y, float width, float height) {
  switch (pivot) {
  case Pivot::TopLeft:
    *this = Rect(x + width * 0.5f, y - height * 0.5f, width, height);
    break;
  case Pivot::TopCenter:
    *this = Rect(x, y - height * 0.5f, width, height);
    break;
  case Pivot::TopRight:
    *this = Rect(x - width * 0.5f, y - height * 0.5f, width, height);
    break;
  case Pivot::Center:
    *this = Rect(x, y, width, height);
    break;
  case Pivot::BottomLeft:
    *this = Rect(x + width * 0.5f, y + height * 0.5f, width, height);
    break;
  case Pivot::BottomCenter:
    *this = Rect(x, y + height * 0.5f, width, height);
    break;
  case Pivot::BottomRight:
    *this = Rect(x - width * 0.5f, y + height * 0.5f, width, height);
    break;
  default:
    assert(false && "Pivot error");
    *this = Rect();
  }
}

Rect::Rect(Pivot pivot, const Vec2 &pos, const Vec2 &size) {
  *this = Rect(pivot, pos.x, pos.y, size.x, size.y);
}

Rect Rect::topLeft() const {
  return Rect(pos.x - size.x * 0.5f, pos.y + size.y * 0.5f, size.x, size.y);
}

Rect Rect::topCenter() const {
  return Rect(pos.x, pos.y + size.y * 0.5f, size.x, size.y);
}

Rect Rect::topRight() const {
  return Rect(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f, size.x, size.y);
}

Rect Rect::center() const { return *this; }
Rect Rect::bottomLeft() const {
  return Rect(pos.x - size.x * 0.5f, pos.y - size.y * 0.5f, size.x, size.y);
}

Rect Rect::bottomCenter() const {
  return Rect(pos.x, pos.y - size.y * 0.5f, size.x, size.y);
}

Rect Rect::bottomRight() const {
  return Rect(pos.x + size.x * 0.5f, pos.y - size.y * 0.5f, size.x, size.y);
}

Rect Rect::fromPivot(Pivot pivot) const {
  switch (pivot) {
  case Pivot::TopLeft:
    return topLeft();
  case Pivot::TopCenter:
    return topCenter();
  case Pivot::TopRight:
    return topRight();
  case Pivot::Center:
    return center();
  case Pivot::BottomLeft:
    return bottomLeft();
  case Pivot::BottomCenter:
    return bottomCenter();
  case Pivot::BottomRight:
    return bottomRight();
  }
  return center();
}

bool Rect::intersectsRect(const Rect &rect) {
  auto own = topLeft();
  auto other = rect.topLeft();
  return (own.x <= other.x + other.width && other.x <= own.x + own.width) &&
         (own.y <= other.y + other.height && other.y <= own.y + other.height);
}

Mesh Rect::createMesh() {
  auto mesh = makePtr<Mesh::Data>();
  mesh->vertices.push_back({Vec3(-1.000000, -1.000000, 0.000000),
                            Vec3(0.000000, 0.000000, 1.000000),
                            Vec2(0.000000, 1.000000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(1.000000, -1.000000, 0.000000),
                            Vec3(0.000000, 0.000000, 1.000000),
                            Vec2(1.000000, 1.000000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(1.000000, 1.000000, 0.000000),
                            Vec3(0.000000, 0.000000, 1.000000),
                            Vec2(1.000000, 0.000000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(-1.000000, 1.000000, 0.000000),
                            Vec3(0.000000, 0.000000, 1.000000),
                            Vec2(0.000000, 0.000000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  uint32_t indices[] = {
      0, 1, 2, 0, 2, 3,
  };
  for (size_t i = 0; i < sizeof(indices) / sizeof(UInt32); i++) {
    mesh->indices.push_back(indices[i]);
  }
  return Mesh{mesh};
}

} // namespace sinen
