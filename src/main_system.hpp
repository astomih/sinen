#include <graphics/drawable/drawable.hpp>

#include <list>
#include <vector>

namespace sinen {
class MainSystem {
public:
  enum class State { running, paused, quit };
  MainSystem() = default;
  ~MainSystem() = default;
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
  static void set_state(const State &state) { m_game_state = state; }
  /**
   * @brief Get the state object
   *
   * @return const scene::state&
   */
  static const State &get_state() { return m_game_state; }
  static void set_run_script(bool is_run) { is_run_script = is_run; }

  static bool is_running() { return m_game_state != State::quit; }
  static bool is_reset_next() { return is_reset; }
  static std::string GetCurrentName() { return m_scene_name; }
  static std::string GetBasePath() { return basePath; }
  static void Change(const std::string &sceneFileName,
                     const std::string &basePath);

  static void process_input();
  static void update_scene();
  static bool is_reset;
  static bool is_run_script;

  static inline float delta_time() { return deltaTime; }

private:
  static State m_game_state;
  static uint32_t m_prev_tick;
  static std::string m_scene_name;
  static std::string basePath;
  static float deltaTime;
};
} // namespace sinen
