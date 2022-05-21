#pragma once
#include "../input/input.hpp"
#include "../script/script.hpp"
#include "../utility/handler.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace nen {
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
  bool isRunning() { return mGameState != game_state::Quit; }

  /**
   * @brief run loop
   *
   */
  void RunLoop();

  /**
   * @brief Get the Renderer object
   *
   */
  void Shutdown();

  /**
   * @brief Get the State object
   *
   * @return game_state
   */
  game_state GetState() const { return mGameState; }
  /**
   * @brief Set the State object
   *
   * @param state
   */
  void SetState(game_state state) { mGameState = state; }

  /**
   * @brief Quit scene
   *
   */
  void Quit() { mGameState = game_state::Quit; }

  void change_scene(std::string scene_name);

protected:
  /**
   * @brief Setup scene
   *
   */
  virtual void Setup();
  /**
   * @brief Update scene
   *
   * @param deltaTime
   */
  virtual void Update(float deltaTime);

private:
  void UnloadData();
  void ProcessInput();
  void UpdateScene();
  game_state mGameState = game_state::Gameplay;
  uint32_t m_prev_tick = 0;
};
} // namespace nen
