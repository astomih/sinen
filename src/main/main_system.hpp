#ifndef SINEN_MANAGER_HPP
#define SINEN_MANAGER_HPP
#include <memory>
#include <string>

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
  void change_scene(std::string scene_number);
  std::string get_current_scene_number() { return m_scene_name; }
  class scene &get_current_scene() { return *m_current_scene; }

private:
  std::unique_ptr<class scene> m_current_scene;
  std::unique_ptr<class scene> m_next_scene;
  void loop();

  std::string m_scene_name = "main";
};
} // namespace sinen

#endif // !SINEN_MANAGER_HPP