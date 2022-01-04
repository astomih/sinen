#pragma once
#include "../Math/Matrix4.hpp"
#include "../Math/Vector3.hpp"
#include "../Math/Quaternion.hpp"
#include "../Component/Component.hpp"
#include "../Input/InputSystem.hpp"
#include <vector>
#include <cstdint>
#include <memory>
#include <type_traits>

namespace nen
{
	class Scene;

	/**
	 * @brief Sceneに依存するアクター
	 * 
	 */
	class Actor
	{
	public:

		/**
		 * @brief アクターの状態を表すステート
		 * 
		 */
		enum class State
		{
			Active,
			Paused,
			Dead
		};

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
		Actor(Scene &scene);
		
		/**
		 * @brief デストラクタ
		 * 
		 */
		virtual ~Actor();
		
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
		const Vector3 &GetPosition() const noexcept { return mPosition; }
		
		/**
		 * @brief 位置ベクトルを設定する
		 * 
		 * @param pos Vector3
		 */
		void SetPosition(const Vector3 &pos) noexcept
		{
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
		void Move(const float x, const float y, const float z) noexcept
		{
			mPosition.x += x;
			mPosition.y += y;
			mPosition.z += z;
			mRecomputeWorldTransform = true;
		}
		
		/**
		 * @brief スケールベクトルを取得
		 * 
		 * @return const Vector3& 
		 */
		const Vector3 &GetScale() const { return mScale; }
		
		/**
		 * @brief スケールベクトルを設定
		 * 
		 * @param scale 
		 */
		void SetScale(const Vector3 &scale)
		{
			mScale = scale;
			mRecomputeWorldTransform = true;
		}
		
		/**
		 * @brief 回転用のクォータニオンを取得
		 * 
		 * @return const Quaternion& 
		 */
		const Quaternion &GetRotation() const { return mRotation; }
		void SetRotation(const Quaternion &rotation)
		{
			mRotation = rotation;
			mRecomputeWorldTransform = true;
		}

		void ComputeWorldTransform();
		const Matrix4 &GetWorldTransform() const { return mWorldTransform; }

		Vector3 GetForward() const { return Vector3::Transform(Vector3::NegUnitZ, mRotation); }

		State GetState() const { return mState; }
		void SetState(State state) { mState = state; }

		Scene &GetScene() { return mScene; }

		template <class T>
		std::shared_ptr<T> GetComponent()
		{
			for (auto i : mComponents)
			{
				auto component = std::dynamic_pointer_cast<T>(i);
				if (component != nullptr)
					return component;
			}
			return nullptr;
		}

		template <class T>
		std::shared_ptr<T> AddComponent(int updateOrder = 100)
		{
			auto com = GetComponent<T>();
			if (com == nullptr)
			{
				auto ptr = std::make_shared<T>(*this, updateOrder);
				if (!mComponents.empty())
				{
					auto iter = mComponents.begin();
					for (; iter != mComponents.end(); ++iter)
					{
						if (updateOrder < (*iter)->GetUpdateOrder())
						{
							break;
						}
					}
					mComponents.insert(iter, ptr);
				}
				else
					mComponents.emplace_back(ptr);
				return ptr;
			}
			else
			{
				RemoveComponent<T>();
				auto ptr = std::make_shared<T>(*this, updateOrder);
				auto iter = mComponents.begin();
				for (; iter != mComponents.end(); ++iter)
				{
					if (updateOrder < (*iter)->GetUpdateOrder())
					{
						break;
					}
				}
				mComponents.insert(iter, ptr);
				return ptr;
			}
		}

		template <class T>
		void RemoveComponent()
		{
			auto iter = std::find(mComponents.begin(), mComponents.end(), GetComponent<T>());
			if (iter != mComponents.end())
			{
				mComponents.erase(iter);
			}
		}

		bool isRecompute() { return mRecomputeWorldTransform; }
		void RecomuteFinished() { mRecomputeWorldTransform = false; }

		const InputState &GetInput();

	protected:
		bool mRecomputeWorldTransform;

	private:
		State mState;
		Matrix4 mWorldTransform;
		Vector3 mPosition;
		Quaternion mRotation;
		std::vector<std::shared_ptr<Component>> mComponents;
		Scene &mScene;
		Vector3 mScale;
	};
}
