#include <graph/bfs_grid.hpp>
#include <iostream>

namespace sinen {
bfs_grid::bfs_grid(const graph &field) {
  this->m_field = field;
  m_dist = graph{width(), height(), -1};
  m_prev_y = m_dist;
  m_prev_x = m_dist;
  reset();
}
void bfs_grid::reset() {
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
bool bfs_grid::find_path(const point2i &start, const point2i &end) {
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
  m_dist[start.y][start.x] = 0;
  queue.push(start);

  // Start finding path
  while (!queue.empty()) {
    point2i current_pos = queue.front();
    int x = current_pos.x;
    int y = current_pos.y;
    queue.pop();

    // Find adjacent vertices
    for (int direction = 0; direction < 4; ++direction) {
      int next_x = x + dx[direction];
      int next_y = y + dy[direction];
      // Next_x or next_y is out of field
      if (next_y < 0 || next_y >= height() || next_x < 0 || next_x >= width())
        continue;
      // Not walkable node
      if (m_field[next_y][next_x] == 1)
        continue;

      // If next node is not visited, set as visited and push to queue
      if (m_dist[next_y][next_x] == -1) {
        queue.push({next_x, next_y});
        // Update distance and previous node
        m_dist[next_y][next_x] = m_dist[y][x] + 1;
        // Update previous node
        m_prev_x[next_y][next_x] = x;
        m_prev_y[next_y][next_x] = y;
        // If next node is end, return true
        if (next_x == end.x && next_y == end.y) {
          backtrace(end);
          return true;
        }
      }
    }
  }
  // If there is no path, return false
  return false;
}
void bfs_grid::backtrace(const point2i &end) {
  point2i p = end;
  while (p.x != -1 && p.y != -1) {
    // Add passed node to shortest_path
    this->shortest.push(p);
    // Backtrack previous node
    int px = m_prev_x[p.y][p.x];
    int py = m_prev_y[p.y][p.x];
    p.x = px, p.y = py;
  }
}
point2i bfs_grid::trace() {
  point2i next = shortest.top();
  shortest.pop();
  return next;
}
bool bfs_grid::traceable() const { return !shortest.empty(); }
} // namespace sinen
