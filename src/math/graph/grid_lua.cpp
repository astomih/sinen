#include <math/graph/grid.hpp>
#include <script/luaapi.hpp>

namespace sinen {
static int lGridNew(lua_State *L) {
  float w = static_cast<float>(luaL_checknumber(L, 1));
  float h = static_cast<float>(luaL_checknumber(L, 2));
  udNewOwned<Grid>(L, Grid(static_cast<size_t>(w), static_cast<size_t>(h)));
  return 1;
}
static int lGridAt(lua_State *L) {
  auto &g = udValue<Grid>(L, 1);
  int x = static_cast<int>(luaL_checkinteger(L, 2));
  int y = static_cast<int>(luaL_checkinteger(L, 3));
  lua_pushnumber(L, g.at(x - 1, y - 1));
  return 1;
}
static int lGridSet(lua_State *L) {
  auto &g = udValue<Grid>(L, 1);
  int x = static_cast<int>(luaL_checkinteger(L, 2));
  int y = static_cast<int>(luaL_checkinteger(L, 3));
  float v = static_cast<float>(luaL_checknumber(L, 4));
  g.at(x - 1, y - 1) = v;
  return 0;
}
static int lGridWidth(lua_State *L) {
  lua_pushinteger(L, static_cast<lua_Integer>(udValue<Grid>(L, 1).width()));
  return 1;
}
static int lGridHeight(lua_State *L) {
  lua_pushinteger(L, static_cast<lua_Integer>(udValue<Grid>(L, 1).height()));
  return 1;
}
static int lGridSize(lua_State *L) {
  lua_pushinteger(L, static_cast<lua_Integer>(udValue<Grid>(L, 1).size()));
  return 1;
}
static int lGridClear(lua_State *L) {
  udValue<Grid>(L, 1).clear();
  return 0;
}
static int lGridResize(lua_State *L) {
  auto &g = udValue<Grid>(L, 1);
  size_t w = static_cast<size_t>(luaL_checkinteger(L, 2));
  size_t h = static_cast<size_t>(luaL_checkinteger(L, 3));
  g.resize(w, h);
  return 0;
}
static int lGridFill(lua_State *L) {
  auto &g = udValue<Grid>(L, 1);
  float v = static_cast<float>(luaL_checknumber(L, 2));
  g.fill(v);
  return 0;
}
static int lGridFillRect(lua_State *L) {
  auto &g = udValue<Grid>(L, 1);
  auto &r = udValue<Rect>(L, 2);
  float v = static_cast<float>(luaL_checknumber(L, 3));
  g.fillRect(r, v);
  return 0;
}
static int lGridSetRow(lua_State *L) {
  auto &g = udValue<Grid>(L, 1);
  int idx = static_cast<int>(luaL_checkinteger(L, 2));
  float v = static_cast<float>(luaL_checknumber(L, 3));
  g.setRow(idx - 1, v);
  return 0;
}
static int lGridSetColumn(lua_State *L) {
  auto &g = udValue<Grid>(L, 1);
  int idx = static_cast<int>(luaL_checkinteger(L, 2));
  float v = static_cast<float>(luaL_checknumber(L, 3));
  g.setColumn(idx - 1, v);
  return 0;
}
void registerGrid(lua_State *L) {
  luaL_newmetatable(L, Grid::metaTableName());
  luaPushcfunction2(L, udGc<Grid>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lGridAt);
  lua_setfield(L, -2, "at");
  luaPushcfunction2(L, lGridSet);
  lua_setfield(L, -2, "set");
  luaPushcfunction2(L, lGridWidth);
  lua_setfield(L, -2, "width");
  luaPushcfunction2(L, lGridHeight);
  lua_setfield(L, -2, "height");
  luaPushcfunction2(L, lGridSize);
  lua_setfield(L, -2, "size");
  luaPushcfunction2(L, lGridClear);
  lua_setfield(L, -2, "clear");
  luaPushcfunction2(L, lGridResize);
  lua_setfield(L, -2, "resize");
  luaPushcfunction2(L, lGridFill);
  lua_setfield(L, -2, "fill");
  luaPushcfunction2(L, lGridFillRect);
  lua_setfield(L, -2, "fillRect");
  luaPushcfunction2(L, lGridSetRow);
  lua_setfield(L, -2, "setRow");
  luaPushcfunction2(L, lGridSetColumn);
  lua_setfield(L, -2, "setColumn");
  lua_pop(L, 1);

  pushSnNamed(L, "Grid");
  luaPushcfunction2(L, lGridNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
