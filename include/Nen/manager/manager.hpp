#ifndef SINEN_MANAGER_HPP
#define SINEN_MANAGER_HPP
#include "../Render/Renderer.hpp"
#include <memory>
namespace nen {
/**
 * @brief sinen manager
 *
 */
class manager {
public:
  /**
   * @brief Construct a new manager object
   *
   */
  manager() = default;
  ~manager() = default;
  manager(const manager &) = delete;
  manager &operator=(const manager &) = delete;
  manager(manager &&) = delete;
  manager &operator=(manager &&) = delete;
  /**
   * @brief initialize manager
   *
   */
  void initialize();

  /**
   * @brief launch engine
   *
   */
  void launch(std::unique_ptr<class base_scene>);
};
} // namespace nen

#endif