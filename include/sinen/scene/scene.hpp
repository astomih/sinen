#ifndef SINEN_SCENE_HPP
#define SINEN_SCENE_HPP
#include "../input/input.hpp"
#include "../script/script.hpp"
#include "../utility/handler.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sinen {
class sound_system;
/**
 * @brief Base of scene class
 *
 */
class scene {
public:
  /**
   * @brief Construct a new base scene object
   *
   */
  scene();
  virtual ~scene() = default;

  /**
   * @brief Game state
   *
   */
  enum class game_state { Gameplay, Paused, Quit };

  /**
   * @brief Initialize scene
   *
   */
  void initialize();

  /**
   * @brief is running scene
   *
   * @return true
   * @return false
   */
  bool is_running() { return mGameState != game_state::Quit; }

  /**
   * @brief run loop
   *
   */
  void run_loop();

  /**
   * @brief Get the Renderer object
   *
   */
  virtual void shutdown();

  /**
   * @brief Get the State object
   *
   * @return game_state
   */
  game_state get_state() const { return mGameState; }
  /**
   * @brief Set the State object
   *
   * @param state
   */
  void set_state(game_state state) { mGameState = state; }

  /**
   * @brief Quit scene
   *
   */
  void quit() { mGameState = game_state::Quit; }
  /**
   * @brief Change scene
   *
   * @param scene_name Scene name
   */
  void change_scene(std::string scene_name);

  /**
   * @brief Reset scene
   *
   */
  void reset();

protected:
  /**
   * @brief Setup scene
   *
   */
  virtual void setup();
  /**
   * @brief Update scene
   *
   * @param deltaTime
   */
  virtual void update(float deltaTime);

private:
  void unload_data();
  void process_input();
  void update_scene();
  game_state mGameState = game_state::Gameplay;
  uint32_t m_prev_tick = 0;
  std::function<void()> m_editor;
  std::function<void()> m_markdown;
};
} // namespace sinen
#endif // !SINEN_SCENE_HPP
