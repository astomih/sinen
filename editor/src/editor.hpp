#ifndef SINEN_EDITOR_HPP
#define SINEN_EDITOR_HPP
#include <memory>
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
  ~editor();
  /**
   * @brief Setup
   *
   */
  void setup() final override;
  /**
   * @brief Update the editor
   *
   * @param delta_time Delta time
   */
  void update(float delta_time) final override;

private:
  class implements;
  std::unique_ptr<implements> m_impl;
  static std::function<void()> m_editor;
  static std::function<void()> m_markdown;
};
} // namespace sinen
#endif
