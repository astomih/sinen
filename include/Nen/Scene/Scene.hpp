#pragma once
#include "../Input/InputSystem.hpp"
#include "../Utility/dynamic_handler.hpp"
#include "Utility/dynamic_handler.hpp"
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
  base_scene() = default;
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
   * @brief Set the Renderer object
   *
   * @param renderer
   */
  void SetRenderer(std::shared_ptr<class renderer> renderer) {
    mRenderer = renderer;
  }

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

  template <class T, typename... _Args>
  T &add_actor(handle_t &handle, _Args &&...__args) {
    return m_actor.add<T>(handle, *this, std::forward<_Args>(__args)...);
  }

  template <class T> T &get_actor(const handle_t &handle) {
    return m_actor.get<T>(handle);
  }

  /**
   * @brief Get the Renderer object
   *
   * @return std::shared_ptr<class renderer>
   */
  std::shared_ptr<class renderer> GetRenderer() const { return mRenderer; }

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

  /**
   * @brief Add GUI element
   *
   * @param ui
   */
  void AddGUI(std::shared_ptr<class ui_screen> ui);
  /**
   * @brief Remove GUI element
   *
   * @param ui
   */
  void RemoveGUI(std::shared_ptr<class ui_screen> ui);

  /**
   * @brief Get the Input object
   *
   * @return const input_state&
   */
  const input_state &GetInput() { return mInputSystem->GetState(); }
  /**
   * @brief Set the Input System object
   *
   * @param inputSystem
   */
  void SetInputSystem(std::shared_ptr<class input_system> inputSystem) {
    mInputSystem = inputSystem;
  }

  /**
   * @brief Get the Sound object
   *
   * @return sound_system&
   */
  sound_system &GetSound() { return *mSoundSystem; }

  /**
   * @brief Set the Sound System object
   *
   * @param soundSystem
   */
  void SetSoundSystem(std::shared_ptr<class sound_system> soundSystem) {
    mSoundSystem = soundSystem;
  }

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
  dynamic_handler<class base_actor> m_actor;
  std::shared_ptr<class input_system> mInputSystem;
  std::shared_ptr<class sound_system> mSoundSystem;
  std::shared_ptr<class renderer> mRenderer;
  game_state mGameState = game_state::Gameplay;
  uint32_t mTicksCount = 0;
};
void ChangeScene(std::unique_ptr<base_scene> newScene);

} // namespace nen
