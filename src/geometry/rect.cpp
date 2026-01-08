#include "rect.hpp"
namespace sinen {
Rect::Rect() : p(), s() {}
Ptr<Mesh> Rect::createMesh() {
  auto mesh = makePtr<Mesh>();
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
  return mesh;
}

} // namespace sinen