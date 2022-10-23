#ifndef SINEN_MANAGER_HPP
#define SINEN_MANAGER_HPP
// std library
#include <memory>
#include <string>
// internal
#include <scene/scene.hpp>
namespace sinen {
/**
 * @brief sinen main system
 *
 */
class main_system {
public:
  /**
   * @brief initialize
   *
   */
  static bool initialize();

  /**
   * @brief launch engine
   *
   */
  static void launch();

  template <class Scene = scene>
  static void change_scene(const std::string &scene_name) {
    change_scene_impl(scene_name);
  }
  static std::string get_current_scene_number() { return m_scene_name; }

private:
  static void change_scene_impl(const std::string &scene_name);
  static bool loop();
  static bool is_reset;
  static std::string m_scene_name;
};
} // namespace sinen

#endif // !SINEN_MANAGER_HPP