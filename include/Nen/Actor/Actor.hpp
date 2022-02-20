#pragma once
#include "../Component/Component.hpp"
#include "../Input/InputSystem.hpp"
#include "../Math/Matrix4.hpp"
#include "../Math/Quaternion.hpp"
#include "../Math/Vector3.hpp"
#include "Component/MoveComponent.hpp"
#include "Utility/dynamic_handler.hpp"
#include <cstdint>
#include <memory>
#include <type_traits>
#include <unordered_map>
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
  base_actor(const base_actor &) = delete;

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
    return vector3::Transform(vector3::UnitY, mRotation);
  }

  state GetState() const { return mState; }
  void SetState(state state) { mState = state; }

  base_scene &GetScene() { return mScene; }

  template <class T, typename... _Args>
  T &add_component(std::uint32_t &handle, _Args &&...__args) {
    return m_components.add<T>(handle, *this, std::forward<_Args>(__args)...);
  }
  template <class T> T &get_component(std::uint32_t handle) {
    return m_components.get<T>(handle);
  }

  void remove_component(std::uint32_t stored_value) {
    m_components.remove(stored_value);
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
  dynamic_handler<base_component> m_components;
  base_scene &mScene;
  vector3 mScale;
}; // namespace nen
} // namespace nen
