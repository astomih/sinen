#ifndef SINEN_BFS_GRID_HPP
#define SINEN_BFS_GRID_HPP
// std library
#include <list>
#include <queue>
#include <stack>
#include <vector>

// internal library
#include "../math/point2.hpp"
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
  bool find_path(const Point2i &start, const Point2i &end);
  /**
   * @brief Trace the shortest path
   *
   * @return point2i Next point
   */
  Point2i trace();
  /**
   * @brief Check if the path is traceable
   *
   * @return true Traceable
   * @return false Not traceable
   */
  bool traceable() const;
  /**
   * @brief Reset the bfs object
   *
   */
  void reset();
  /**
   * @brief Get the height of the field
   *
   * @return std::size_t
   */
  std::size_t width() const noexcept { return this->m_field.width(); }
  /**
   * @brief Get the width of the field
   *
   * @return std::size_t
   */
  std::size_t height() const noexcept { return this->m_field.height(); }

private:
  /**
   * @brief Backtrace the shortest path
   *
   * @param end Trace end point
   */
  void backtrace(const Point2i &end);
  graph m_field;
  graph m_dist;
  graph m_prev_x;
  graph m_prev_y;
  std::stack<Point2i> shortest;
  std::queue<Point2i> queue;
};
} // namespace sinen
#endif // !SINEN_BFS_HPP