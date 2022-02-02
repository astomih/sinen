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
 * @brief Sceneに依存するアクター
 *
 */
class base_actor {
public:
  /**
   * @brief アクターの状態を表すステート
   *
   */
  enum class state { Active, Paused, Dead };

  /**
   * @brief 識別用のハンドル
   *
   */
  uint32_t handle;

  /**
   * @brief コンストラクタ
   *
   * @param scene 親シーンの参照
   */
  base_actor(base_scene &scene);

  /**
   * @brief デストラクタ
   *
   */
  virtual ~base_actor();

  /**
   * @brief アップデート時に呼ばれる関数
   *
   * @param deltaTime 変位時間
   */
  void UpdateActor(float deltaTime);

  /**
   * @brief 毎ループ更新される関数
   *
   * @param deltaTime 変位時間
   */
  virtual void Update(float deltaTime);

  /**
   * @brief アクターごとのIDを取得
   *
   * @return std::string 文字列
   */
  virtual std::string GetID() { return ""; }

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

  template <class T> std::shared_ptr<T> GetComponent() {
    for (auto i : mComponents) {
      auto component = std::dynamic_pointer_cast<T>(i);
      if (component != nullptr)
        return component;
    }
    return nullptr;
  }

  template <class T> std::shared_ptr<T> AddComponent(int updateOrder = 100) {
    auto com = GetComponent<T>();
    if (com == nullptr) {
      auto ptr = std::make_shared<T>(*this, updateOrder);
      if (!mComponents.empty()) {
        auto iter = mComponents.begin();
        for (; iter != mComponents.end(); ++iter) {
          if (updateOrder < (*iter)->GetUpdateOrder()) {
            break;
          }
        }
        mComponents.insert(iter, ptr);
      } else
        mComponents.emplace_back(ptr);
      return ptr;
    } else {
      RemoveComponent<T>();
      auto ptr = std::make_shared<T>(*this, updateOrder);
      auto iter = mComponents.begin();
      for (; iter != mComponents.end(); ++iter) {
        if (updateOrder < (*iter)->GetUpdateOrder()) {
          break;
        }
      }
      mComponents.insert(iter, ptr);
      return ptr;
    }
  }

  template <class T> void RemoveComponent() {
    auto iter =
        std::find(mComponents.begin(), mComponents.end(), GetComponent<T>());
    if (iter != mComponents.end()) {
      mComponents.erase(iter);
    }
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
  std::vector<std::shared_ptr<base_component>> mComponents;
  base_scene &mScene;
  vector3 mScale;
};
} // namespace nen
