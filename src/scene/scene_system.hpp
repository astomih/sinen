#include <drawable/drawable_wrapper.hpp>
#include <scene/scene.hpp>

#include <list>
#include <vector>

namespace sinen {
class scene_system {
public:
  scene_system() = default;
  ~scene_system() = default;
  /**
   * @brief Initialize scene system
   *
   * @return true success
   * @return false failed
   */
  static bool initialize();
  /**
   * @brief Terminate scene system
   *
   */
  static void shutdown();
  static void setup();
  /**
   * @brief Set the state object
   *
   * @param state
   */
  static void set_state(const Scene::state &state) { m_game_state = state; }
  /**
   * @brief Get the state object
   *
   * @return const scene::state&
   */
  static const Scene::state &get_state() { return m_game_state; }
  static void set_run_script(bool is_run) { is_run_script = is_run; }

  static bool is_running() { return m_game_state != Scene::state::quit; }
  static bool is_reset_next() { return is_reset; }
  static std::string current_name() { return m_scene_name; }
  static void change(const std::string &scene_file_name);
  static void change_impl(std::unique_ptr<Scene::implements> impl) {
    m_impl = std::move(impl);
  }

  static void process_input();
  static void update_scene();
  static bool is_reset;

private:
  static bool is_run_script;
  static std::unique_ptr<Scene::implements> m_impl;
  static Scene::state m_game_state;
  static uint32_t m_prev_tick;
  static std::string m_scene_name;
};
} // namespace sinen
