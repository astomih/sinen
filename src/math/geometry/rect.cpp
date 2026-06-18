#include <math/geometry/rect.hpp>

#include <cstring>
namespace sinen {
Rect::Rect() : _pos(), _size() {}

Rect::Rect(float x, float y, float width, float height)
    : x(x), y(y), width(width), height(height) {}

Rect::Rect(const Vec2 &_pos, const Vec2 &_size) : _pos(_pos), _size(_size) {}

Rect::Rect(Pivot pivot, float x, float y, float width, float height) {
  switch (pivot) {
  case Pivot::TopLeft:
    *this = Rect(x, y, width, height);
    break;
  case Pivot::TopCenter:
    *this = Rect(x - width * 0.5f, y, width, height);
    break;
  case Pivot::TopRight:
    *this = Rect(x - width, y, width, height);
    break;
  case Pivot::Left:
    *this = Rect(x, y - height * 0.5f, width, height);
    break;
  case Pivot::Center:
    *this = Rect(x - width * 0.5f, y - height * 0.5f, width, height);
    break;
  case Pivot::Right:
    *this = Rect(x - width, y - height * 0.5f, width, height);
    break;
  case Pivot::BottomLeft:
    *this = Rect(x, y - height, width, height);
    break;
  case Pivot::BottomCenter:
    *this = Rect(x - width * 0.5f, y - height, width, height);
    break;
  case Pivot::BottomRight:
    *this = Rect(x - width, y - height, width, height);
    break;
  default:
    assert(false && "Invalid pivot");
    *this = Rect();
  }
}

Rect::Rect(Pivot pivot, const Vec2 &pos, const Vec2 &size) {
  *this = Rect(pivot, pos.x, pos.y, size.x, size.y);
}

Vec2 Rect::topLeft() const { return _pos; }

Vec2 Rect::topCenter() const { return Vec2(_pos.x + _size.x * 0.5f, _pos.y); }

Vec2 Rect::topRight() const { return Vec2(_pos.x + _size.x, _pos.y); }
Vec2 Rect::left() const { return Vec2(_pos.x, _pos.y + _size.y * 0.5f); }
Vec2 Rect::center() const {
  return Vec2(_pos.x + _size.x * 0.5f, _pos.y + _size.y * 0.5f);
}
Vec2 Rect::right() const {
  return Vec2(_pos.x + _size.x, _pos.y + _size.y * 0.5f);
}
Vec2 Rect::bottomLeft() const { return Vec2(_pos.x, _pos.y + _size.y); }

Vec2 Rect::bottomCenter() const {
  return Vec2(_pos.x + _size.x * 0.5f, _pos.y + _size.y);
}

Vec2 Rect::bottomRight() const {
  return Vec2(_pos.x + _size.x, _pos.y + _size.y);
}

Vec2 Rect::positionfromPivot(Pivot pivot) const {
  switch (pivot) {
  case Pivot::TopLeft:
    return topLeft();
  case Pivot::TopCenter:
    return topCenter();
  case Pivot::TopRight:
    return topRight();
  case Pivot::Left:
    return left();
  case Pivot::Center:
    return center();
  case Pivot::Right:
    return right();
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
  return (x <= rect.x + rect.width && rect.x <= x + width) &&
         (y <= rect.y + rect.height && rect.y <= y + height);
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
