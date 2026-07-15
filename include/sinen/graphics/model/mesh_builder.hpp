#ifndef SINEN_MESH_BUILDER_HPP
#define SINEN_MESH_BUILDER_HPP

#include <math/color/color.hpp>
#include <math/geometry/mesh.hpp>
#include <math/vector.hpp>

namespace sinen {
class MeshBuilder {
public:
  static constexpr const char *metaTableName() { return "sn.MeshBuilder"; }

  void clear();
  UInt32 addVertex(const Vec3 &position,
                   const Vec3 &normal = Vec3(0.0f, 1.0f, 0.0f),
                   const Vec2 &uv = Vec2(0.0f),
                   const Color &color = Color(1.0f));
  void addTriangle(UInt32 a, UInt32 b, UInt32 c);
  void addQuad(UInt32 a, UInt32 b, UInt32 c, UInt32 d);

  void addPlane(float width = 1.0f, float depth = 1.0f,
                const Color &color = Color(1.0f));
  void addBox(const Vec3 &size = Vec3(1.0f), const Color &color = Color(1.0f));
  void addSphere(float radius = 1.0f, UInt32 rings = 16, UInt32 segments = 32,
                 const Color &color = Color(1.0f));
  void addCylinder(float radius = 1.0f, float height = 2.0f,
                   UInt32 segments = 32, const Color &color = Color(1.0f));
  void addCone(float radius = 1.0f, float height = 2.0f, UInt32 segments = 32,
               const Color &color = Color(1.0f));

  void recalculateNormals();
  Mesh toMesh() const;
  UInt32 vertexCount() const;
  UInt32 indexCount() const;

private:
  Mesh mesh;
};
} // namespace sinen

#endif // SINEN_MESH_BUILDER_HPP
