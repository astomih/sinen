#include <script/luaapi.hpp>
#include <graphics/model/mesh_builder.hpp>
#include <math/color/color.hpp>
#include <graphics/model/model.hpp>

namespace sinen {
namespace {
Color optionalColor(lua_State *L, int idx) {
  if (lua_gettop(L) < idx || lua_isnil(L, idx)) {
    return Color(1.0f);
  }
  return udValue<Color>(L, idx);
}

UInt32 checkOneBasedIndex(lua_State *L, int idx, UInt32 count) {
  lua_Integer value = luaL_checkinteger(L, idx);
  if (value <= 0 || static_cast<UInt32>(value) > count) {
    luaLError2(L, "sn.MeshBuilder: vertex index %d is out of range",
               static_cast<int>(value));
    return 0;
  }
  return static_cast<UInt32>(value - 1);
}
} // namespace

static int lMeshBuilderNew(lua_State *L) {
  udNewOwned<MeshBuilder>(L, MeshBuilder{});
  return 1;
}

static int lMeshBuilderClear(lua_State *L) {
  udValue<MeshBuilder>(L, 1).clear();
  return 0;
}

static int lMeshBuilderVertex(lua_State *L) {
  auto &b = udValue<MeshBuilder>(L, 1);
  auto &position = udValue<Vec3>(L, 2);
  Vec3 normal(0.0f, 1.0f, 0.0f);
  Vec2 uv(0.0f);
  Color color(1.0f);
  if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
    normal = udValue<Vec3>(L, 3);
  }
  if (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) {
    uv = udValue<Vec2>(L, 4);
  }
  if (lua_gettop(L) >= 5 && !lua_isnil(L, 5)) {
    color = udValue<Color>(L, 5);
  }
  lua_pushinteger(L, b.addVertex(position, normal, uv, color));
  return 1;
}

static int lMeshBuilderTriangle(lua_State *L) {
  auto &b = udValue<MeshBuilder>(L, 1);
  const UInt32 count = b.vertexCount();
  const UInt32 a = checkOneBasedIndex(L, 2, count);
  const UInt32 c = checkOneBasedIndex(L, 3, count);
  const UInt32 d = checkOneBasedIndex(L, 4, count);
  b.addTriangle(a, c, d);
  return 0;
}

static int lMeshBuilderQuad(lua_State *L) {
  auto &b = udValue<MeshBuilder>(L, 1);
  const UInt32 count = b.vertexCount();
  const UInt32 a = checkOneBasedIndex(L, 2, count);
  const UInt32 c = checkOneBasedIndex(L, 3, count);
  const UInt32 d = checkOneBasedIndex(L, 4, count);
  const UInt32 e = checkOneBasedIndex(L, 5, count);
  b.addQuad(a, c, d, e);
  return 0;
}

static int lMeshBuilderAddPlane(lua_State *L) {
  auto &b = udValue<MeshBuilder>(L, 1);
  const float width = static_cast<float>(luaL_optnumber(L, 2, 1.0));
  const float depth = static_cast<float>(luaL_optnumber(L, 3, 1.0));
  b.addPlane(width, depth, optionalColor(L, 4));
  return 0;
}

static int lMeshBuilderAddBox(lua_State *L) {
  auto &b = udValue<MeshBuilder>(L, 1);
  Vec3 size(1.0f);
  Color color(1.0f);
  if (auto *v = udValueOrNull<Vec3>(L, 2)) {
    size = *v;
    color = optionalColor(L, 3);
  } else if (lua_gettop(L) >= 4) {
    size = Vec3(static_cast<float>(luaL_checknumber(L, 2)),
                static_cast<float>(luaL_checknumber(L, 3)),
                static_cast<float>(luaL_checknumber(L, 4)));
    color = optionalColor(L, 5);
  } else if (lua_gettop(L) >= 2 && lua_isnumber(L, 2)) {
    size = Vec3(static_cast<float>(luaL_checknumber(L, 2)));
    color = optionalColor(L, 3);
  }
  b.addBox(size, color);
  return 0;
}

static int lMeshBuilderAddSphere(lua_State *L) {
  auto &b = udValue<MeshBuilder>(L, 1);
  const float radius = static_cast<float>(luaL_optnumber(L, 2, 1.0));
  const UInt32 rings = static_cast<UInt32>(luaL_optinteger(L, 3, 16));
  const UInt32 segments = static_cast<UInt32>(luaL_optinteger(L, 4, 32));
  b.addSphere(radius, rings, segments, optionalColor(L, 5));
  return 0;
}

static int lMeshBuilderAddCylinder(lua_State *L) {
  auto &b = udValue<MeshBuilder>(L, 1);
  const float radius = static_cast<float>(luaL_optnumber(L, 2, 1.0));
  const float height = static_cast<float>(luaL_optnumber(L, 3, 2.0));
  const UInt32 segments = static_cast<UInt32>(luaL_optinteger(L, 4, 32));
  b.addCylinder(radius, height, segments, optionalColor(L, 5));
  return 0;
}

static int lMeshBuilderAddCone(lua_State *L) {
  auto &b = udValue<MeshBuilder>(L, 1);
  const float radius = static_cast<float>(luaL_optnumber(L, 2, 1.0));
  const float height = static_cast<float>(luaL_optnumber(L, 3, 2.0));
  const UInt32 segments = static_cast<UInt32>(luaL_optinteger(L, 4, 32));
  b.addCone(radius, height, segments, optionalColor(L, 5));
  return 0;
}

static int lMeshBuilderRecalculateNormals(lua_State *L) {
  udValue<MeshBuilder>(L, 1).recalculateNormals();
  return 0;
}

static int lMeshBuilderToModel(lua_State *L) {
  auto &b = udValue<MeshBuilder>(L, 1);
  auto model = makePtr<Model>();
  model->loadFromVertexArray(b.toMesh());
  udPushPtr<Model>(L, model);
  return 1;
}

static int lMeshBuilderVertexCount(lua_State *L) {
  lua_pushinteger(L, udValue<MeshBuilder>(L, 1).vertexCount());
  return 1;
}

static int lMeshBuilderIndexCount(lua_State *L) {
  lua_pushinteger(L, udValue<MeshBuilder>(L, 1).indexCount());
  return 1;
}

void registerMeshBuilder(lua_State *L) {
  luaL_newmetatable(L, MeshBuilder::metaTableName());
  luaPushcfunction2(L, udGc<MeshBuilder>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lMeshBuilderClear);
  lua_setfield(L, -2, "clear");
  luaPushcfunction2(L, lMeshBuilderVertex);
  lua_setfield(L, -2, "vertex");
  luaPushcfunction2(L, lMeshBuilderTriangle);
  lua_setfield(L, -2, "triangle");
  luaPushcfunction2(L, lMeshBuilderQuad);
  lua_setfield(L, -2, "quad");
  luaPushcfunction2(L, lMeshBuilderAddPlane);
  lua_setfield(L, -2, "addPlane");
  luaPushcfunction2(L, lMeshBuilderAddBox);
  lua_setfield(L, -2, "addBox");
  luaPushcfunction2(L, lMeshBuilderAddSphere);
  lua_setfield(L, -2, "addSphere");
  luaPushcfunction2(L, lMeshBuilderAddCylinder);
  lua_setfield(L, -2, "addCylinder");
  luaPushcfunction2(L, lMeshBuilderAddCone);
  lua_setfield(L, -2, "addCone");
  luaPushcfunction2(L, lMeshBuilderRecalculateNormals);
  lua_setfield(L, -2, "recalculateNormals");
  luaPushcfunction2(L, lMeshBuilderToModel);
  lua_setfield(L, -2, "toModel");
  luaPushcfunction2(L, lMeshBuilderVertexCount);
  lua_setfield(L, -2, "vertexCount");
  luaPushcfunction2(L, lMeshBuilderIndexCount);
  lua_setfield(L, -2, "indexCount");
  lua_pop(L, 1);

  pushSnNamed(L, "MeshBuilder");
  luaPushcfunction2(L, lMeshBuilderNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
