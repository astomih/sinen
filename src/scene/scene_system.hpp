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
   * @brief Update scene system
   *
   */
  static void run_loop();
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
  static void set_state(const scene::state &state) { m_game_state = state; }
  /**
   * @brief Get the state object
   *
   * @return const scene::state&
   */
  static const scene::state &get_state() { return m_game_state; }
  static void set_run_script(bool is_run) { is_run_script = is_run; }

  static bool is_running() { return m_game_state != scene::state::quit; }
  static void change_impl(std::unique_ptr<scene::implements> impl) {
    m_impl = std::move(impl);
  }

  static void add_actor(actor *_actor);
  static void load_data(std::string_view data_file_name);

private:
  static bool is_run_script;
  static std::unique_ptr<scene::implements> m_impl;
  using actor_ptr = actor *;
  static std::vector<actor_ptr> m_actors;
  static void process_input();
  static void update_scene();
  static scene::state m_game_state;
  static uint32_t m_prev_tick;
};
} // namespace sinen
