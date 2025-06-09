#ifndef SINEN_BFS_GRID_HPP
#define SINEN_BFS_GRID_HPP
// std library
#include <list>
#include <queue>
#include <stack>
#include <vector>

// external library
#include <glm/vec2.hpp>

// internal library
#include "glm/ext/vector_int2.hpp"
#include "grid.hpp"

namespace sinen {
/**
 * @brief Breadth First Search Algorithm
 *
 */
class BFSGrid {
public:
  using graph = Grid<int>;
  /**
   * @brief Construct a new bfs object
   *
   * @param field trace field
   */
  BFSGrid(const graph &field);
  /**
   * @brief Destroy the bfs object
   *
   */
  ~BFSGrid() = default;
  /**
   * @brief Find the shortest path from start to end
   *
   * @param start
   * @param end
   * @return true
   * @return false
   */
  bool FindPath(const glm::ivec2 &start, const glm::ivec2 &end);
  /**
   * @brief Trace the shortest path
   *
   * @return point2i Next point
   */
  glm::ivec2 Trace();
  /**
   * @brief Check if the path is traceable
   *
   * @return true Traceable
   * @return false Not traceable
   */
  bool Traceable() const;
  /**
   * @brief Reset the bfs object
   *
   */
  void Reset();
  /**
   * @brief Get the height of the field
   *
   * @return std::size_t
   */
  std::size_t Width() const noexcept { return this->m_field.Width(); }
  /**
   * @brief Get the width of the field
   *
   * @return std::size_t
   */
  std::size_t Height() const noexcept { return this->m_field.Height(); }

private:
  /**
   * @brief Backtrace the shortest path
   *
   * @param end Trace end point
   */
  void backtrace(const glm::ivec2 &end);
  graph m_field;
  graph m_dist;
  graph m_prev_x;
  graph m_prev_y;
  std::stack<glm::ivec2> shortest;
  std::queue<glm::ivec2> queue;
};
} // namespace sinen
#endif // !SINEN_BFS_HPP