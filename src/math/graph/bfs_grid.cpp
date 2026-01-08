#include <math/graph/bfs_grid.hpp>

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
} // namespace sinen
