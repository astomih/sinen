#pragma once
#include "../Input/InputSystem.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace nen {
class sound_system;
class base_scene {
public:
  base_scene();
  virtual ~base_scene() {}

  enum class GameState { Gameplay, Paused, Quit };

  void Initialize();

  void SetRenderer(std::shared_ptr<class renderer> renderer) {
    mRenderer = renderer;
  }

  bool isRunning() { return mGameState != GameState::Quit; }

  void RunLoop();

  void Shutdown();

  template <class T> std::shared_ptr<T> AddActor() {
    auto actor = std::make_shared<T>(*this);
    if (mUpdatingActors) {
      mPendingActors.emplace_back(actor);
    } else {
      mActors.emplace_back(actor);
    }
    return actor;
  }
  template <class T, typename... _Args>
  T &add_actor2(std::uint16_t &store_value, _Args &&...__args) {
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
  template <class T> T &get_actor2(std::uint16_t stored_value) {
    auto it = m_actor_map.find(stored_value);
    if (it == m_actor_map.end()) {
      throw std::runtime_error("actor not found");
    }
    base_actor *ptr = it->second.get();
    return *static_cast<T *>(ptr);
  }

  template <class T> std::shared_ptr<T> GetActor(uint32_t handle = 0) {
    for (const auto &i : mActors) {
      auto actor = std::dynamic_pointer_cast<T>(i);
      if (actor && i->handle == handle)
        return actor;
    }
    for (const auto &i : mPendingActors) {
      auto actor = std::dynamic_pointer_cast<T>(i);
      if (actor && i->handle == handle)
        return actor;
    }
    return nullptr;
  }

  template <class T> void RemoveActor(std::shared_ptr<T> actor) {
    auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
    if (iter != mPendingActors.end()) {
      std::iter_swap(iter, mPendingActors.end() - 1);
      mPendingActors.pop_back();
    }

    iter = std::find(mActors.begin(), mActors.end(), actor);
    if (iter != mActors.end()) {
      std::iter_swap(iter, mActors.end() - 1);
      mActors.pop_back();
    }
  }

  std::shared_ptr<class renderer> GetRenderer() const { return mRenderer; }

  GameState GetState() const { return mGameState; }
  void SetState(GameState state) { mGameState = state; }

  void Quit() { mGameState = GameState::Quit; }

  void AddGUI(std::shared_ptr<class ui_screen> ui);
  void RemoveGUI(std::shared_ptr<class ui_screen> ui);

  const input_state &GetInput() { return mInputSystem->GetState(); }
  void SetInputSystem(std::shared_ptr<class input_system> inputSystem) {
    mInputSystem = inputSystem;
  }

  sound_system &GetSound() { return *mSoundSystem; }
  void SetSoundSystem(std::shared_ptr<class sound_system> soundSystem) {
    mSoundSystem = soundSystem;
  }

protected:
  virtual void Setup();
  virtual void Update(float deltaTime);

  // All the actors in the game
  std::vector<std::shared_ptr<class base_actor>> mActors;

private:
  void UnloadData();
  void ProcessInput();
  void UpdateScene();
  std::unordered_map<std::uint16_t, std::unique_ptr<class base_actor>>
      m_actor_map;
  std::unordered_map<std::uint16_t, std::unique_ptr<class base_actor>>
      m_pending_actor_map;
  std::shared_ptr<class input_system> mInputSystem;
  std::shared_ptr<class sound_system> mSoundSystem;
  std::shared_ptr<class renderer> mRenderer;
  std::vector<std::shared_ptr<class base_actor>> mPendingActors;
  GameState mGameState = GameState::Gameplay;
  uint32_t mTicksCount = 0;
  std::uint16_t m_next_handle = 0;
  bool mUpdatingActors = false;
};
void ChangeScene(std::shared_ptr<base_scene> newScene);

void Launch(std::shared_ptr<base_scene> scene);
} // namespace nen
