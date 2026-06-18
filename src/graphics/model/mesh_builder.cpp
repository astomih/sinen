#include <graphics/model/mesh_builder.hpp>

#include <graphics/model/model.hpp>

namespace sinen {
namespace {
constexpr float kEpsilon = 0.000001f;

Vec3 safeNormalize(const Vec3 &v, const Vec3 &fallback) {
  if (v.lengthSqrt() <= kEpsilon) {
    return fallback;
  }
  return Vec3::normalize(v);
}

} // namespace

void MeshBuilder::clear() { mesh = Mesh(); }

UInt32 MeshBuilder::addVertex(const Vec3 &position, const Vec3 &normal,
                              const Vec2 &uv, const Color &color) {
  auto data = mesh.data();
  data->vertices.push_back(Vertex(
      position, safeNormalize(normal, Vec3(0.0f, 1.0f, 0.0f)), uv, color));
  return static_cast<UInt32>(data->vertices.size());
}

void MeshBuilder::addTriangle(UInt32 a, UInt32 b, UInt32 c) {
  auto data = mesh.data();
  data->indices.push_back(a);
  data->indices.push_back(b);
  data->indices.push_back(c);
}

void MeshBuilder::addQuad(UInt32 a, UInt32 b, UInt32 c, UInt32 d) {
  addTriangle(a, b, c);
  addTriangle(a, c, d);
}

void MeshBuilder::addPlane(float width, float depth, const Color &color) {
  const float hx = width * 0.5f;
  const float hz = depth * 0.5f;
  const UInt32 base = vertexCount();
  const Vec3 n(0.0f, 1.0f, 0.0f);

  addVertex(Vec3(-hx, 0.0f, -hz), n, Vec2(0.0f, 1.0f), color);
  addVertex(Vec3(hx, 0.0f, -hz), n, Vec2(1.0f, 1.0f), color);
  addVertex(Vec3(hx, 0.0f, hz), n, Vec2(1.0f, 0.0f), color);
  addVertex(Vec3(-hx, 0.0f, hz), n, Vec2(0.0f, 0.0f), color);
  addQuad(base + 0, base + 1, base + 2, base + 3);
}

void MeshBuilder::addBox(const Vec3 &size, const Color &color) {
  const Vec3 h = size * 0.5f;
  struct Face {
    Vec3 normal;
    Vec3 p[4];
  };
  const Face faces[] = {
      {Vec3(0.0f, 0.0f, 1.0f),
       {Vec3(-h.x, -h.y, h.z), Vec3(h.x, -h.y, h.z), Vec3(h.x, h.y, h.z),
        Vec3(-h.x, h.y, h.z)}},
      {Vec3(0.0f, 0.0f, -1.0f),
       {Vec3(h.x, -h.y, -h.z), Vec3(-h.x, -h.y, -h.z), Vec3(-h.x, h.y, -h.z),
        Vec3(h.x, h.y, -h.z)}},
      {Vec3(1.0f, 0.0f, 0.0f),
       {Vec3(h.x, -h.y, h.z), Vec3(h.x, -h.y, -h.z), Vec3(h.x, h.y, -h.z),
        Vec3(h.x, h.y, h.z)}},
      {Vec3(-1.0f, 0.0f, 0.0f),
       {Vec3(-h.x, -h.y, -h.z), Vec3(-h.x, -h.y, h.z), Vec3(-h.x, h.y, h.z),
        Vec3(-h.x, h.y, -h.z)}},
      {Vec3(0.0f, 1.0f, 0.0f),
       {Vec3(-h.x, h.y, h.z), Vec3(h.x, h.y, h.z), Vec3(h.x, h.y, -h.z),
        Vec3(-h.x, h.y, -h.z)}},
      {Vec3(0.0f, -1.0f, 0.0f),
       {Vec3(-h.x, -h.y, -h.z), Vec3(h.x, -h.y, -h.z), Vec3(h.x, -h.y, h.z),
        Vec3(-h.x, -h.y, h.z)}},
  };

  for (const auto &face : faces) {
    const UInt32 base = vertexCount();
    addVertex(face.p[0], face.normal, Vec2(0.0f, 1.0f), color);
    addVertex(face.p[1], face.normal, Vec2(1.0f, 1.0f), color);
    addVertex(face.p[2], face.normal, Vec2(1.0f, 0.0f), color);
    addVertex(face.p[3], face.normal, Vec2(0.0f, 0.0f), color);
    addQuad(base + 0, base + 1, base + 2, base + 3);
  }
}

void MeshBuilder::addSphere(float radius, UInt32 rings, UInt32 segments,
                            const Color &color) {
  rings = Math::max<UInt32>(rings, 2);
  segments = Math::max<UInt32>(segments, 3);
  const UInt32 base = vertexCount();

  for (UInt32 y = 0; y <= rings; ++y) {
    const float v = static_cast<float>(y) / static_cast<float>(rings);
    const float phi = v * Math::pi;
    const float sinPhi = Math::sin(phi);
    const float cosPhi = Math::cos(phi);
    for (UInt32 x = 0; x <= segments; ++x) {
      const float u = static_cast<float>(x) / static_cast<float>(segments);
      const float theta = u * Math::twoPi;
      Vec3 normal(Math::sin(theta) * sinPhi, cosPhi, Math::cos(theta) * sinPhi);
      addVertex(normal * radius, normal, Vec2(u, v), color);
    }
  }

  const UInt32 stride = segments + 1;
  for (UInt32 y = 0; y < rings; ++y) {
    for (UInt32 x = 0; x < segments; ++x) {
      const UInt32 a = base + y * stride + x;
      const UInt32 b = base + (y + 1) * stride + x;
      const UInt32 c = base + (y + 1) * stride + x + 1;
      const UInt32 d = base + y * stride + x + 1;
      addQuad(a, b, c, d);
    }
  }
}

void MeshBuilder::addCylinder(float radius, float height, UInt32 segments,
                              const Color &color) {
  segments = Math::max<UInt32>(segments, 3);
  const float hy = height * 0.5f;
  const UInt32 sideBase = vertexCount();

  for (UInt32 i = 0; i <= segments; ++i) {
    const float u = static_cast<float>(i) / static_cast<float>(segments);
    const float angle = u * Math::twoPi;
    Vec3 normal(Math::sin(angle), 0.0f, Math::cos(angle));
    Vec3 lower = normal * radius;
    Vec3 upper = lower;
    lower.y = -hy;
    upper.y = hy;
    addVertex(lower, normal, Vec2(u, 1.0f), color);
    addVertex(upper, normal, Vec2(u, 0.0f), color);
  }

  for (UInt32 i = 0; i < segments; ++i) {
    const UInt32 a = sideBase + i * 2;
    addQuad(a, a + 2, a + 3, a + 1);
  }

  const UInt32 topCenter =
      addVertex(Vec3(0.0f, hy, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec2(0.5f),
                color) -
      1;
  const UInt32 bottomCenter =
      addVertex(Vec3(0.0f, -hy, 0.0f), Vec3(0.0f, -1.0f, 0.0f), Vec2(0.5f),
                color) -
      1;
  for (UInt32 i = 0; i < segments; ++i) {
    const UInt32 next = i + 1;
    const float u0 = static_cast<float>(i) / static_cast<float>(segments);
    const float u1 = static_cast<float>(next) / static_cast<float>(segments);
    const float a0 = u0 * Math::twoPi;
    const float a1 = u1 * Math::twoPi;
    Vec3 p0(Math::sin(a0) * radius, hy, Math::cos(a0) * radius);
    Vec3 p1(Math::sin(a1) * radius, hy, Math::cos(a1) * radius);
    Vec3 q0(p0.x, -hy, p0.z);
    Vec3 q1(p1.x, -hy, p1.z);
    const UInt32 t0 = addVertex(p0, Vec3(0.0f, 1.0f, 0.0f),
                                Vec2(0.5f + Math::sin(a0) * 0.5f,
                                     0.5f + Math::cos(a0) * 0.5f),
                                color) -
                      1;
    const UInt32 t1 = addVertex(p1, Vec3(0.0f, 1.0f, 0.0f),
                                Vec2(0.5f + Math::sin(a1) * 0.5f,
                                     0.5f + Math::cos(a1) * 0.5f),
                                color) -
                      1;
    const UInt32 b0 = addVertex(q0, Vec3(0.0f, -1.0f, 0.0f),
                                Vec2(0.5f + Math::sin(a0) * 0.5f,
                                     0.5f + Math::cos(a0) * 0.5f),
                                color) -
                      1;
    const UInt32 b1 = addVertex(q1, Vec3(0.0f, -1.0f, 0.0f),
                                Vec2(0.5f + Math::sin(a1) * 0.5f,
                                     0.5f + Math::cos(a1) * 0.5f),
                                color) -
                      1;
    addTriangle(topCenter, t0, t1);
    addTriangle(bottomCenter, b1, b0);
  }
}

void MeshBuilder::addCone(float radius, float height, UInt32 segments,
                          const Color &color) {
  segments = Math::max<UInt32>(segments, 3);
  const float hy = height * 0.5f;
  const Vec3 apex(0.0f, hy, 0.0f);

  for (UInt32 i = 0; i < segments; ++i) {
    const float u0 = static_cast<float>(i) / static_cast<float>(segments);
    const float u1 = static_cast<float>(i + 1) / static_cast<float>(segments);
    const float a0 = u0 * Math::twoPi;
    const float a1 = u1 * Math::twoPi;
    Vec3 p0(Math::sin(a0) * radius, -hy, Math::cos(a0) * radius);
    Vec3 p1(Math::sin(a1) * radius, -hy, Math::cos(a1) * radius);
    const Vec3 normal =
        safeNormalize(Vec3::cross(p1 - p0, apex - p0), Vec3(0.0f, 1.0f, 0.0f));
    const UInt32 base = vertexCount();
    addVertex(p0, normal, Vec2(u0, 1.0f), color);
    addVertex(p1, normal, Vec2(u1, 1.0f), color);
    addVertex(apex, normal, Vec2((u0 + u1) * 0.5f, 0.0f), color);
    addTriangle(base + 0, base + 1, base + 2);
  }

  const UInt32 center = addVertex(Vec3(0.0f, -hy, 0.0f),
                                  Vec3(0.0f, -1.0f, 0.0f), Vec2(0.5f), color) -
                        1;
  for (UInt32 i = 0; i < segments; ++i) {
    const float a0 =
        static_cast<float>(i) / static_cast<float>(segments) * Math::twoPi;
    const float a1 =
        static_cast<float>(i + 1) / static_cast<float>(segments) * Math::twoPi;
    Vec3 p0(Math::sin(a0) * radius, -hy, Math::cos(a0) * radius);
    Vec3 p1(Math::sin(a1) * radius, -hy, Math::cos(a1) * radius);
    const UInt32 b0 = addVertex(p0, Vec3(0.0f, -1.0f, 0.0f),
                                Vec2(0.5f + Math::sin(a0) * 0.5f,
                                     0.5f + Math::cos(a0) * 0.5f),
                                color) -
                      1;
    const UInt32 b1 = addVertex(p1, Vec3(0.0f, -1.0f, 0.0f),
                                Vec2(0.5f + Math::sin(a1) * 0.5f,
                                     0.5f + Math::cos(a1) * 0.5f),
                                color) -
                      1;
    addTriangle(center, b1, b0);
  }
}

void MeshBuilder::recalculateNormals() {
  auto data = mesh.data();
  Array<Vec3> normals(data->vertices.size(), Vec3(0.0f));
  for (Size i = 0; i + 2 < data->indices.size(); i += 3) {
    const UInt32 ia = data->indices[i + 0];
    const UInt32 ib = data->indices[i + 1];
    const UInt32 ic = data->indices[i + 2];
    if (ia >= data->vertices.size() || ib >= data->vertices.size() ||
        ic >= data->vertices.size()) {
      continue;
    }
    const Vec3 &a = data->vertices[ia].position;
    const Vec3 &b = data->vertices[ib].position;
    const Vec3 &c = data->vertices[ic].position;
    const Vec3 n = Vec3::cross(b - a, c - a);
    normals[ia] += n;
    normals[ib] += n;
    normals[ic] += n;
  }
  for (Size i = 0; i < data->vertices.size(); ++i) {
    data->vertices[i].normal =
        safeNormalize(normals[i], data->vertices[i].normal);
  }
}

Mesh MeshBuilder::toMesh() const { return Mesh(*mesh.data()); }

UInt32 MeshBuilder::vertexCount() const {
  return static_cast<UInt32>(mesh.data()->vertices.size());
}

UInt32 MeshBuilder::indexCount() const {
  return static_cast<UInt32>(mesh.data()->indices.size());
}

} // namespace sinen
