#include "bbox.hpp"
namespace sinen {

void AABB::updateWorld(const Vec3 &p, const Vec3 &scale, const AABB &local) {
  this->min = p + scale * local.min;
  this->max = p + scale * local.max;
}

Mesh AABB::createMesh() {
  auto mesh = makePtr<Mesh::Data>();
  mesh->vertices.push_back({Vec3(1.000000, 1.000000, 1.000000),
                            Vec3(0.000000, 0.000000, 1.000000),
                            Vec2(0.625000, 0.500000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(-1.000000, 1.000000, 1.000000),
                            Vec3(0.000000, 0.000000, 1.000000),
                            Vec2(0.875000, 0.500000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(-1.000000, -1.000000, 1.000000),
                            Vec3(0.000000, 0.000000, 1.000000),
                            Vec2(0.875000, 0.250000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(1.000000, -1.000000, 1.000000),
                            Vec3(0.000000, 0.000000, 1.000000),
                            Vec2(0.625000, 0.250000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(1.000000, -1.000000, -1.000000),
                            Vec3(0.000000, -1.000000, 0.000000),
                            Vec2(0.375000, 0.250000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(1.000000, -1.000000, 1.000000),
                            Vec3(0.000000, -1.000000, 0.000000),
                            Vec2(0.625000, 0.250000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(-1.000000, -1.000000, 1.000000),
                            Vec3(0.000000, -1.000000, 0.000000),
                            Vec2(0.625000, 0.000000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(-1.000000, -1.000000, -1.000000),
                            Vec3(0.000000, -1.000000, 0.000000),
                            Vec2(0.375000, 0.000000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(-1.000000, -1.000000, -1.000000),
                            Vec3(-1.000000, 0.000000, 0.000000),
                            Vec2(0.375000, 1.000000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(-1.000000, -1.000000, 1.000000),
                            Vec3(-1.000000, 0.000000, 0.000000),
                            Vec2(0.625000, 1.000000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(-1.000000, 1.000000, 1.000000),
                            Vec3(-1.000000, 0.000000, 0.000000),
                            Vec2(0.625000, 0.750000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(-1.000000, 1.000000, -1.000000),
                            Vec3(-1.000000, 0.000000, 0.000000),
                            Vec2(0.375000, 0.750000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(-1.000000, 1.000000, -1.000000),
                            Vec3(0.000000, 0.000000, -1.000000),
                            Vec2(0.125000, 0.500000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(1.000000, 1.000000, -1.000000),
                            Vec3(0.000000, 0.000000, -1.000000),
                            Vec2(0.375000, 0.500000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(1.000000, -1.000000, -1.000000),
                            Vec3(0.000000, 0.000000, -1.000000),
                            Vec2(0.375000, 0.250000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(-1.000000, -1.000000, -1.000000),
                            Vec3(0.000000, 0.000000, -1.000000),
                            Vec2(0.125000, 0.250000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(1.000000, 1.000000, -1.000000),
                            Vec3(1.000000, 0.000000, 0.000000),
                            Vec2(0.375000, 0.500000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(1.000000, 1.000000, 1.000000),
                            Vec3(1.000000, 0.000000, 0.000000),
                            Vec2(0.625000, 0.500000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(1.000000, -1.000000, 1.000000),
                            Vec3(1.000000, 0.000000, 0.000000),
                            Vec2(0.625000, 0.250000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(1.000000, -1.000000, -1.000000),
                            Vec3(1.000000, 0.000000, 0.000000),
                            Vec2(0.375000, 0.250000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(-1.000000, 1.000000, -1.000000),
                            Vec3(0.000000, 1.000000, 0.000000),
                            Vec2(0.375000, 0.750000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(-1.000000, 1.000000, 1.000000),
                            Vec3(0.000000, 1.000000, 0.000000),
                            Vec2(0.625000, 0.750000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(1.000000, 1.000000, 1.000000),
                            Vec3(0.000000, 1.000000, 0.000000),
                            Vec2(0.625000, 0.500000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({Vec3(1.000000, 1.000000, -1.000000),
                            Vec3(0.000000, 1.000000, 0.000000),
                            Vec2(0.375000, 0.500000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  uint32_t indices[] = {
      0,  1,  2,  0,  2,  3,  4,  5,  6,  4,  6,  7,  8,  9,  10, 8,  10, 11,
      12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23,
  };
  for (Size i = 0; i < sizeof(indices) / sizeof(UInt32); i++) {
    mesh->indices.push_back(indices[i]);
  }
  return Mesh{mesh};
}
bool AABB::intersectsAABB(const AABB &aabb) const {
  const auto &a = *this;
  const auto &b = aabb;

  return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
         (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
         (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

static float segmentLengthOnSeparateAxis(const Vec3 &Sep, const Vec3 &e1,
                                         const Vec3 &e2, const Vec3 &e3) {
  float r1 = Math::abs(Vec3::dot(Sep, e1));
  float r2 = Math::abs(Vec3::dot(Sep, e2));
  float r3 = Math::abs(Vec3::dot(Sep, e3));
  return r1 + r2 + r3;
}

static float segmentLengthOnSeparateAxis(const Vec3 &Sep, const Vec3 &e1,
                                         const Vec3 &e2) {
  float r1 = Math::abs(Vec3::dot(Sep, e1));
  float r2 = Math::abs(Vec3::dot(Sep, e2));
  return r1 + r2;
}

bool OBB::intersectsOBB(const OBB &obb) const {
  const auto &a = *this;
  const auto &b = obb;

  Vec3 nae1 = a.v[0], ae1 = nae1 * a.a[0];
  Vec3 nae2 = a.v[1], ae2 = nae2 * a.a[1];
  Vec3 nae3 = a.v[2], ae3 = nae3 * a.a[2];
  Vec3 nbe1 = b.v[0], be1 = nbe1 * b.a[0];
  Vec3 nbe2 = b.v[1], be2 = nbe2 * b.a[1];
  Vec3 nbe3 = b.v[2], be3 = nbe3 * b.a[2];
  Vec3 interval = a.p - b.p;
  // Ae1
  float rA = ae1.length();
  float rB = segmentLengthOnSeparateAxis(nae1, be1, be2, be3);
  float l = Math::abs(Vec3::dot(interval, nae1));
  if (l > rA + rB)
    return false;

  // Ae2
  rA = ae2.length();
  rB = segmentLengthOnSeparateAxis(nae2, be1, be2, be3);
  l = Math::abs(Vec3::dot(interval, nae2));
  if (l > rA + rB)
    return false;

  // Ae3
  rA = ae3.length();
  rB = segmentLengthOnSeparateAxis(nae3, be1, be2, be3);
  l = Math::abs(Vec3::dot(interval, nae3));
  if (l > rA + rB)
    return false;

  // Be1
  rA = segmentLengthOnSeparateAxis(nbe1, ae1, ae2, ae3);
  rB = be1.length();
  l = Math::abs(Vec3::dot(interval, nbe1));
  if (l > rA + rB)
    return false;

  // Be2
  rA = segmentLengthOnSeparateAxis(nbe2, ae1, ae2, ae3);
  rB = be2.length();
  l = Math::abs(Vec3::dot(interval, nbe2));
  if (l > rA + rB)
    return false;

  // Be3
  rA = segmentLengthOnSeparateAxis(nbe3, ae1, ae2, ae3);
  rB = be3.length();
  l = Math::abs(Vec3::dot(interval, nbe3));
  if (l > rA + rB)
    return false;

  Vec3 cross;
  // C11
  cross = Vec3::cross(nae1, nbe1);
  rA = segmentLengthOnSeparateAxis(cross, ae2, ae3);
  rB = segmentLengthOnSeparateAxis(cross, be2, be3);
  l = Math::abs(Vec3::dot(interval, cross));
  if (l > rA + rB)
    return false;

  // C12
  cross = Vec3::cross(nae1, nbe2);
  rA = segmentLengthOnSeparateAxis(cross, ae2, ae3);
  rB = segmentLengthOnSeparateAxis(cross, be1, be3);
  l = Math::abs(Vec3::dot(interval, cross));
  if (l > rA + rB)
    return false;

  // C13
  cross = Vec3::cross(nae1, nbe3);
  rA = segmentLengthOnSeparateAxis(cross, ae2, ae3);
  rB = segmentLengthOnSeparateAxis(cross, be1, be2);
  l = Math::abs(Vec3::dot(interval, cross));
  if (l > rA + rB)
    return false;

  // C21
  cross = Vec3::cross(nae2, nbe1);
  rA = segmentLengthOnSeparateAxis(cross, ae1, ae3);
  rB = segmentLengthOnSeparateAxis(cross, be2, be3);
  l = Math::abs(Vec3::dot(interval, cross));
  if (l > rA + rB)
    return false;

  // C22
  cross = Vec3::cross(nae2, nbe2);
  rA = segmentLengthOnSeparateAxis(cross, ae1, ae3);
  rB = segmentLengthOnSeparateAxis(cross, be1, be3);
  l = Math::abs(Vec3::dot(interval, cross));
  if (l > rA + rB)
    return false;

  // C23
  cross = Vec3::cross(nae2, nbe3);
  rA = segmentLengthOnSeparateAxis(cross, ae1, ae3);
  rB = segmentLengthOnSeparateAxis(cross, be1, be2);
  l = Math::abs(Vec3::dot(interval, cross));
  if (l > rA + rB)
    return false;

  // C31
  cross = Vec3::cross(nae3, nbe1);
  rA = segmentLengthOnSeparateAxis(cross, ae1, ae2);
  rB = segmentLengthOnSeparateAxis(cross, be2, be3);
  l = Math::abs(Vec3::dot(interval, cross));
  if (l > rA + rB)
    return false;

  // C32
  cross = Vec3::cross(nae3, nbe2);
  rA = segmentLengthOnSeparateAxis(cross, ae1, ae2);
  rB = segmentLengthOnSeparateAxis(cross, be1, be3);
  l = Math::abs(Vec3::dot(interval, cross));
  if (l > rA + rB)
    return false;

  // C33
  cross = Vec3::cross(nae3, nbe3);
  rA = segmentLengthOnSeparateAxis(cross, ae1, ae2);
  rB = segmentLengthOnSeparateAxis(cross, be1, be2);
  l = Math::abs(Vec3::dot(interval, cross));
  if (l > rA + rB)
    return false;

  // no separating hyperplane theorem found, so the two OBBs are intersecting
  return true;
}

} // namespace sinen
#include <script/luaapi.hpp>
namespace sinen {

static int lAabbNew(lua_State *L) {
  udNewOwned<AABB>(L, AABB{});
  return 1;
}
static int lAabbIndex(lua_State *L) {
  auto &aabb = udValue<AABB>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "min") == 0) {
    udNewRef<Vec3>(L, &aabb.min);
    return 1;
  }
  if (std::strcmp(k, "max") == 0) {
    udNewRef<Vec3>(L, &aabb.max);
    return 1;
  }
  luaL_getmetatable(L, AABB::metaTableName());
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lAabbNewindex(lua_State *L) {
  auto &aabb = udValue<AABB>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  auto &v = udValue<Vec3>(L, 3);
  if (std::strcmp(k, "min") == 0) {
    aabb.min = v;
    return 0;
  }
  if (std::strcmp(k, "max") == 0) {
    aabb.max = v;
    return 0;
  }
  return luaLError2(L, "sn.AABB: invalid field '%s'", k);
}
static int lAabbUpdateWorld(lua_State *L) {
  auto &aabb = udValue<AABB>(L, 1);
  auto &p = udValue<Vec3>(L, 2);
  auto &scale = udValue<Vec3>(L, 3);
  auto &local = udValue<AABB>(L, 4);
  aabb.updateWorld(p, scale, local);
  return 0;
}
static int lAabbIntersectsAabb(lua_State *L) {
  auto &a = udValue<AABB>(L, 1);
  auto &b = udValue<AABB>(L, 2);
  lua_pushboolean(L, a.intersectsAABB(b));
  return 1;
}
void registerAABB(lua_State *L) {
  luaL_newmetatable(L, AABB::metaTableName());
  luaPushcfunction2(L, udGc<AABB>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lAabbIndex);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lAabbNewindex);
  lua_setfield(L, -2, "__newindex");
  luaPushcfunction2(L, lAabbUpdateWorld);
  lua_setfield(L, -2, "updateWorld");
  luaPushcfunction2(L, lAabbIntersectsAabb);
  lua_setfield(L, -2, "intersectsAABB");
  lua_pop(L, 1);

  pushSnNamed(L, "AABB");
  luaPushcfunction2(L, lAabbNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen