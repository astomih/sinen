// std
#include "glm/ext/vector_int2.hpp"
#include <iostream>

// internal
#include <math/graph/bfs_grid.hpp>

namespace sinen {
BFSGrid::BFSGrid(const graph &field) {
  this->m_field = field;
  m_dist = graph{Width(), Height(), -1};
  m_prev_y = m_dist;
  m_prev_x = m_dist;
  Reset();
}
void BFSGrid::Reset() {
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
bool BFSGrid::FindPath(const glm::ivec2 &start, const glm::ivec2 &end) {
  constexpr int dx[4] = {0, 1, 0, -1};
  constexpr int dy[4] = {1, 0, -1, 0};
  // start or end is not in field
  if (start.x < 0 || start.x >= Width() || start.y < 0 || start.y >= Height()) {
    return false;
  }
  if (end.x < 0 || end.x >= Width() || end.y < 0 || end.y >= Height()) {
    return false;
  }
  // Start position set as visited
  m_dist[start.y][start.x] = 0;
  queue.push(start);

  // Start finding path
  while (!queue.empty()) {
    glm::ivec2 current_pos = queue.front();
    int x = current_pos.x;
    int y = current_pos.y;
    queue.pop();

    // Find adjacent vertices
    for (int direction = 0; direction < 4; ++direction) {
      int next_x = x + dx[direction];
      int next_y = y + dy[direction];
      // Next_x or next_y is out of field
      if (next_y < 0 || next_y >= Height() || next_x < 0 || next_x >= Width())
        continue;
      // Not walkable node
      if (m_field[next_y][next_x] < 0)
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
void BFSGrid::backtrace(const glm::ivec2 &end) {
  auto p = end;
  while (p.x != -1 && p.y != -1) {
    // Add passed node to shortest_path
    this->shortest.push(p);
    // Backtrack previous node
    int px = m_prev_x[p.y][p.x];
    int py = m_prev_y[p.y][p.x];
    p.x = px, p.y = py;
  }
}
glm::ivec2 BFSGrid::Trace() {
  glm::ivec2 next = shortest.top();
  shortest.pop();
  return next;
}
bool BFSGrid::Traceable() const { return !shortest.empty(); }
} // namespace sinen
