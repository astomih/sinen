#pragma once
#include "../Input/InputSystem.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
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

  /**
   * @brief Add actor to scene
   *
   * @tparam T actor type
   * @tparam _Args arguments
   * @param store_value store value
   * @param __args arguments
   * @return T& Actor object
   */
  template <class T, typename... _Args>
  T &add_actor(std::uint32_t &store_value = m_default_handle,
               _Args &&...__args) {
    auto actor = std::make_unique<T>(*this, std::forward<_Args>(__args)...);
    store_value = m_next_handle++;
    if (mUpdatingActors) {
      m_pending_actor_map.emplace(store_value, std::move(actor));
    } else {
      m_actor_map.emplace(store_value, std::move(actor));
    }
    base_actor *ptr = m_actor_map[store_value].get();
    return *static_cast<T *>(ptr);
  }
  /**
   * @brief Get the actor object
   *
   * @tparam T actor type
   * @param stored_value
   * @return T& Actor object
   */
  template <class T> T &get_actor(std::uint32_t stored_value) {
    auto it = m_actor_map.find(stored_value);
    if (it == m_actor_map.end()) {
      throw std::runtime_error("actor not found");
    }
    base_actor *ptr = it->second.get();
    return *static_cast<T *>(ptr);
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
  /**
   * @brief Unload scene data
   *
   */
  void UnloadData();
  /**
   * @brief Process Input
   *
   */
  void ProcessInput();
  /**
   * @brief Update Scene impl
   *
   */
  void UpdateScene();
  /**
   * @brief actor map
   *
   */
  std::unordered_map<std::uint32_t, std::unique_ptr<class base_actor>>
      m_actor_map;
  /**
   * @brief pending actor map
   *
   */
  std::unordered_map<std::uint32_t, std::unique_ptr<class base_actor>>
      m_pending_actor_map;
  /**
   * @brief input system
   *
   */
  std::shared_ptr<class input_system> mInputSystem;
  /**
   * @brief sound system
   *
   */
  std::shared_ptr<class sound_system> mSoundSystem;
  /**
   * @brief renderer
   *
   */
  std::shared_ptr<class renderer> mRenderer;
  /**
   * @brief state
   *
   */
  game_state mGameState = game_state::Gameplay;
  /**
   * @brief tick count
   *
   */
  uint32_t mTicksCount = 0;
  /**
   * @brief next handle
   *
   */
  std::uint32_t m_next_handle = 0;

  /**
   * @brief Default handler
   *
   */
  static std::uint32_t m_default_handle;
  /**
   * @brief is updating actors
   *
   */
  bool mUpdatingActors = false;
};
/**
 * @brief Change scene
 *
 * @param newScene
 */
void ChangeScene(std::unique_ptr<base_scene> newScene);

} // namespace nen
