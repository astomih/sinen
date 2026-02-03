#include <math/graph/bfs_grid.hpp>
#include <script/luaapi.hpp>

namespace sinen {
BFSGrid::BFSGrid(const graph &field) {
  this->m_field = field;
  m_dist = graph{width(), height(), -1};
  m_prev_y = m_dist;
  m_prev_x = m_dist;
  reset();
}
void BFSGrid::reset() {
  for (auto &i : m_dist) {
    i = -1;
  }
  for (auto &i : m_prev_y) {
    i = -1;
  }
  for (auto &i : m_prev_x) {
    i = -1;
  }
  while (!shortest.empty()) {
    shortest.pop();
  }
  while (!queue.empty()) {
    queue.pop();
  }
}
bool BFSGrid::findPath(const Vec2 &start, const Vec2 &end) {
  constexpr int dx[4] = {0, 1, 0, -1};
  constexpr int dy[4] = {1, 0, -1, 0};
  // start or end is not in field
  if (start.x < 0 || start.x >= width() || start.y < 0 || start.y >= height()) {
    return false;
  }
  if (end.x < 0 || end.x >= width() || end.y < 0 || end.y >= height()) {
    return false;
  }
  // Start position set as visited
  m_dist[static_cast<int>(start.y)][static_cast<int>(start.x)] = 0.f;
  queue.push(start);

  // Start finding path
  while (!queue.empty()) {
    Vec2 currentPos = queue.front();
    int x = currentPos.x;
    int y = currentPos.y;
    queue.pop();

    // Find adjacent vertices
    for (int direction = 0; direction < 4; ++direction) {
      int nextX = x + dx[direction];
      int nextY = y + dy[direction];
      // Next_x or next_y is out of field
      if (nextY < 0 || nextY >= height() || nextX < 0 || nextX >= width())
        continue;
      // Not walkable node
      if (m_field[nextY][nextX] < 0)
        continue;

      // If next node is not visited, set as visited and push to queue
      if (m_dist[nextY][nextX] == -1) {
        queue.push({static_cast<float>(nextX), static_cast<float>(nextY)});
        // Update distance and previous node
        m_dist[nextY][nextX] = m_dist[y][x] + 1;
        // Update previous node
        m_prev_x[nextY][nextX] = x;
        m_prev_y[nextY][nextX] = y;
        // If next node is end, return true
        if (nextX == end.x && nextY == end.y) {
          backtrace(end);
          return true;
        }
      }
    }
  }
  // If there is no path, return false
  return false;
}
void BFSGrid::backtrace(const Vec2 &end) {
  auto p = end;
  while (p.x != -1 && p.y != -1) {
    // Add passed node to shortest_path
    this->shortest.push(p);
    // Backtrack previous node
    int px = m_prev_x[static_cast<int>(p.y)][static_cast<int>(p.x)];
    int py = m_prev_y[static_cast<int>(p.y)][static_cast<int>(p.x)];
    p.x = px, p.y = py;
  }
}
Vec2 BFSGrid::trace() {
  Vec2 next = shortest.top();
  shortest.pop();
  return next;
}
bool BFSGrid::traceable() const { return !shortest.empty(); }

static int lBfsGridNew(lua_State *L) {
  auto &g = udValue<Grid>(L, 1);
  udNewOwned<BFSGrid>(L, BFSGrid(g));
  return 1;
}
static int lBfsGridWidth(lua_State *L) {
  lua_pushinteger(L, static_cast<lua_Integer>(udValue<BFSGrid>(L, 1).width()));
  return 1;
}
static int lBfsGridHeight(lua_State *L) {
  lua_pushinteger(L, static_cast<lua_Integer>(udValue<BFSGrid>(L, 1).height()));
  return 1;
}
static int lBfsGridFindPath(lua_State *L) {
  auto &b = udValue<BFSGrid>(L, 1);
  auto &start = udValue<Vec2>(L, 2);
  auto &end = udValue<Vec2>(L, 3);
  lua_pushboolean(L, b.findPath(start, end));
  return 1;
}
static int lBfsGridTrace(lua_State *L) {
  udNewOwned<Vec2>(L, udValue<BFSGrid>(L, 1).trace());
  return 1;
}
static int lBfsGridTraceable(lua_State *L) {
  lua_pushboolean(L, udValue<BFSGrid>(L, 1).traceable());
  return 1;
}
static int lBfsGridReset(lua_State *L) {
  udValue<BFSGrid>(L, 1).reset();
  return 0;
}
void registerBFSGrid(lua_State *L) {
  luaL_newmetatable(L, BFSGrid::metaTableName());
  luaPushcfunction2(L, udGc<BFSGrid>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lBfsGridWidth);
  lua_setfield(L, -2, "width");
  luaPushcfunction2(L, lBfsGridHeight);
  lua_setfield(L, -2, "height");
  luaPushcfunction2(L, lBfsGridFindPath);
  lua_setfield(L, -2, "findPath");
  luaPushcfunction2(L, lBfsGridTrace);
  lua_setfield(L, -2, "trace");
  luaPushcfunction2(L, lBfsGridTraceable);
  lua_setfield(L, -2, "traceable");
  luaPushcfunction2(L, lBfsGridReset);
  lua_setfield(L, -2, "reset");
  lua_pop(L, 1);

  pushSnNamed(L, "BFSGrid");
  luaPushcfunction2(L, lBfsGridNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
