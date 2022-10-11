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
   * @brief Construct a new main system object
   *
   */
  main_system() = default;
  ~main_system() = default;
  main_system(const main_system &) = delete;
  /**
   * @brief initialize
   *
   */
  bool initialize();

  /**
   * @brief launch engine
   *
   */
  void launch();

  class scene &get_scene() { return *m_scene; }
  template <class Scene = scene>
  void change_scene(const std::string &scene_name) {
    change_scene_impl(scene_name);
  }
  template <class Scene = scene> void change_base_scene() {
    m_scene = std::make_unique<Scene>();
  }
  std::string get_current_scene_number() { return m_scene_name; }

private:
  void change_scene_impl(const std::string &scene_name);
  std::unique_ptr<class scene> m_scene;
  bool is_reset = false;
  void loop();

  std::string m_scene_name = "main";
};
} // namespace sinen

#endif // !SINEN_MANAGER_HPP