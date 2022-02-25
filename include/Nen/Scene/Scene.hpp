#pragma once
#include "../Input/InputSystem.hpp"
#include "../Utility/handler.hpp"
#include "Script/Script.hpp"
#include "Utility/handle_t.hpp"
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
class base_scene {
public:
  /**
   * @brief Construct a new base scene object
   *
   */
  base_scene(class manager &_manager);
  virtual ~base_scene() = default;

  /**
   * @brief Game state
   *
   */
  enum class game_state { Gameplay, Paused, Quit };

  /**
   * @brief Initialize scene
   *
   */
  void Initialize();

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

  class renderer &GetRenderer();
  const input_state &GetInput();
  class sound_system &GetSound();
  class script_system &get_script();
  class texture_system &get_texture();
  class font_system &get_font();
  void change_scene(std::uint32_t scene_number);

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
  class manager &m_manager;
  game_state mGameState = game_state::Gameplay;
  uint32_t m_prev_tick = 0;
};
} // namespace nen
