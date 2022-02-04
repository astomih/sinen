#pragma once
#include "../Component/Component.hpp"
#include "../Input/InputSystem.hpp"
#include "../Math/Matrix4.hpp"
#include "../Math/Quaternion.hpp"
#include "../Math/Vector3.hpp"
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

namespace nen {
class base_scene;

/**
 * @brief Base of all actors.
 *
 */
class base_actor {
public:
  /**
   * @brief State of the actor
   *
   */
  enum class state { Active, Paused, Dead };

  /**
   * @brief Construct a new base actor object
   *
   * @param scene Parent scene
   */
  base_actor(base_scene &scene);

  /**
   * @brief Destroy the base actor object
   *
   */
  virtual ~base_actor();

  /**
   * @brief Update the actor
   *
   * @param deltaTime dT
   */
  void UpdateActor(float deltaTime);

  /**
   * @brief 毎ループ更新される関数
   *
   * @param deltaTime 変位時間
   */
  virtual void Update(float deltaTime);

  /**
   * @brief 位置ベクトルを取得する
   *
   * @return const Vector3&
   */
  const vector3 &GetPosition() const noexcept { return mPosition; }

  /**
   * @brief 位置ベクトルを設定する
   *
   * @param pos Vector3
   */
  void SetPosition(const vector3 &pos) noexcept {
    mPosition = pos;
    mRecomputeWorldTransform = true;
  }

  /**
   * @brief 現在の位置ベクトルに加算する
   *
   * @param x
   * @param y
   * @param z
   */
  void Move(const float x, const float y, const float z) noexcept {
    mPosition.x += x;
    mPosition.y += y;
    mPosition.z += z;
    mRecomputeWorldTransform = true;
  }

  void Move(vector3 vec) noexcept {
    mPosition += vec;
    mRecomputeWorldTransform = true;
  }

  /**
   * @brief スケールベクトルを取得
   *
   * @return const Vector3&
   */
  const vector3 &GetScale() const { return mScale; }

  /**
   * @brief スケールベクトルを設定
   *
   * @param scale
   */
  void SetScale(const vector3 &scale) {
    mScale = scale;
    mRecomputeWorldTransform = true;
  }

  /**
   * @brief 回転用のクォータニオンを取得
   *
   * @return const Quaternion&
   */
  const quaternion &GetRotation() const { return mRotation; }
  void SetRotation(const quaternion &rotation) {
    mRotation = rotation;
    mRecomputeWorldTransform = true;
  }

  void ComputeWorldTransform();
  const matrix4 &GetWorldTransform() const { return mWorldTransform; }

  vector3 GetForward() const {
    return vector3::Transform(vector3::NegUnitZ, mRotation);
  }

  state GetState() const { return mState; }
  void SetState(state state) { mState = state; }

  base_scene &GetScene() { return mScene; }

  template <class T, typename... _Args>
  T &add_component(std::uint32_t &store_value = m_default_handle,
                   _Args &&...__args) {
    auto component = std::make_unique<T>(*this, std::forward<_Args>(__args)...);
    store_value = m_next_handle++;
    m_components.emplace(store_value, std::move(component));
    base_component *ptr = m_components[store_value].get();
    return *static_cast<T *>(ptr);
  }
  template <class T> T &get_component(std::uint32_t stored_value) {
    auto it = m_components.find(stored_value);
    if (it == m_components.end()) {
      throw std::runtime_error("component not found");
    }
    base_component *ptr = it->second.get();
    return *static_cast<T *>(ptr);
  }

  bool isRecompute() { return mRecomputeWorldTransform; }
  void RecomuteFinished() { mRecomputeWorldTransform = false; }

  const input_state &GetInput();

protected:
  bool mRecomputeWorldTransform;

private:
  state mState;
  matrix4 mWorldTransform;
  vector3 mPosition;
  quaternion mRotation;
  std::unordered_map<std::uint32_t, std::unique_ptr<base_component>>
      m_components;
  base_scene &mScene;
  vector3 mScale;
  static std::uint32_t m_default_handle;
  std::uint32_t m_next_handle;
};
} // namespace nen
