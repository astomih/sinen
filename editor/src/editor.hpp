#ifndef SINEN_EDITOR_HPP
#define SINEN_EDITOR_HPP
#include <sinen/scene/scene.hpp>

namespace sinen {
/**
 * @brief Editor
 *
 */
class editor : public scene::implements {
public:
  /**
   * @brief Construct a new editor object
   *
   */
  editor();
  /**
   * @brief Destroy the editor object
   *
   */
  ~editor() override;
  /**
   * @brief Setup
   *
   */
  void setup() override;
  /**
   * @brief Update the editor
   *
   * @param delta_time Delta time
   */
  void update(float delta_time) override;
};
} // namespace sinen
#endif