#include "rect.hpp"
#include <script/luaapi.hpp>

#include <cstring>
namespace sinen {
Rect::Rect() : _pos(), _size() {}

Rect::Rect(float x, float y, float width, float height)
    : x(x), y(y), width(width), height(height) {}

Rect::Rect(const Vec2 &_pos, const Vec2 &_size) : _pos(_pos), _size(_size) {}

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
    assert(false && "Invalid pivot");
    *this = Rect();
  }
}

Rect::Rect(Pivot pivot, const Vec2 &pos, const Vec2 &size) {
  *this = Rect(pivot, pos.x, pos.y, size.x, size.y);
}

Vec2 Rect::topLeft() const {
  return Vec2(_pos.x - _size.x * 0.5f, _pos.y + _size.y * 0.5f);
}

Vec2 Rect::topCenter() const { return Vec2(_pos.x, _pos.y + _size.y * 0.5f); }

Vec2 Rect::topRight() const {
  return Vec2(_pos.x + _size.x * 0.5f, _pos.y + _size.y * 0.5f);
}

Vec2 Rect::center() const { return _pos; }
Vec2 Rect::bottomLeft() const {
  return Vec2(_pos.x - _size.x * 0.5f, _pos.y - _size.y * 0.5f);
}

Vec2 Rect::bottomCenter() const {
  return Vec2(_pos.x, _pos.y - _size.y * 0.5f);
}

Vec2 Rect::bottomRight() const {
  return Vec2(_pos.x + _size.x * 0.5f, _pos.y - _size.y * 0.5f);
}

Vec2 Rect::positionfromPivot(Pivot pivot) const {
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
  return (own.x <= other.x + width && other.x <= own.x + width) &&
         (other.y <= own.y + height && own.y <= other.y + height);
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

static int lRectNew(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned<Rect>(L, Rect());
    return 1;
  }
  if (n == 4) {
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    float w = static_cast<float>(luaL_checknumber(L, 3));
    float h = static_cast<float>(luaL_checknumber(L, 4));
    udNewOwned<Rect>(L, Rect(x, y, w, h));
    return 1;
  }
  if (n == 2) {
    auto &pos = udValue<Vec2>(L, 1);
    auto &size = udValue<Vec2>(L, 2);
    udNewOwned<Rect>(L, Rect(pos, size));
    return 1;
  }
  if (n == 5) {
    Pivot pivot = static_cast<Pivot>(luaL_checkinteger(L, 1));
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));
    float w = static_cast<float>(luaL_checknumber(L, 4));
    float h = static_cast<float>(luaL_checknumber(L, 5));
    udNewOwned<Rect>(L, Rect(pivot, x, y, w, h));
    return 1;
  }
  if (n == 3) {
    Pivot pivot = static_cast<Pivot>(luaL_checkinteger(L, 1));
    auto &pos = udValue<Vec2>(L, 2);
    auto &size = udValue<Vec2>(L, 3);
    udNewOwned<Rect>(L, Rect(pivot, pos, size));
    return 1;
  }
  return luaLError2(L, "sn.Rect.new: invalid arguments");
}

static int lRectIndex(lua_State *L) {
  auto &r = udValue<Rect>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "x") == 0) {
    lua_pushnumber(L, r.x);
    return 1;
  }
  if (std::strcmp(k, "y") == 0) {
    lua_pushnumber(L, r.y);
    return 1;
  }
  if (std::strcmp(k, "width") == 0) {
    lua_pushnumber(L, r.width);
    return 1;
  }
  if (std::strcmp(k, "height") == 0) {
    lua_pushnumber(L, r.height);
    return 1;
  }
  luaL_getmetatable(L, Rect::metaTableName());
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lRectNewindex(lua_State *L) {
  auto &r = udValue<Rect>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  float v = static_cast<float>(luaL_checknumber(L, 3));
  if (std::strcmp(k, "x") == 0) {
    r.x = v;
    return 0;
  }
  if (std::strcmp(k, "y") == 0) {
    r.y = v;
    return 0;
  }
  if (std::strcmp(k, "width") == 0) {
    r.width = v;
    return 0;
  }
  if (std::strcmp(k, "height") == 0) {
    r.height = v;
    return 0;
  }
  return luaLError2(L, "sn.Rect: invalid field '%s'", k);
}
void registerRect(lua_State *L) {
  luaL_newmetatable(L, Rect::metaTableName());
  luaPushcfunction2(L, udGc<Rect>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lRectIndex);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lRectNewindex);
  lua_setfield(L, -2, "__newindex");
  lua_pop(L, 1);

  pushSnNamed(L, "Rect");
  luaPushcfunction2(L, lRectNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

} // namespace sinen
