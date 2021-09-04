#pragma once
#include "../Math/Matrix4.hpp"
#include "../Math/Vector3.hpp"
#include "../Math/Quaternion.hpp"
#include "../Component/Component.hpp"
#include <vector>
#include <cstdint>
#include <memory>
#include <type_traits>

namespace nen
{
	class Scene;

	class Actor : public std::enable_shared_from_this<Actor>
	{
	public:
		enum class State
		{
			Active,
			Paused,
			Dead
		};
		uint32_t handle;
		Actor(Scene &scene);
		virtual ~Actor();
		void Update(float deltaTime);
		void UpdateComponents(float deltaTime);
		virtual void UpdateActor(float deltaTime);
		virtual std::string GetID() { return ""; }

		void ProcessInput(const struct InputState &state);
		virtual void ActorInput(const struct InputState &state);

		const Vector3 &GetPosition() const noexcept { return mPosition; }
		void SetPosition(const Vector3 &pos) noexcept
		{
			mPosition = pos;
			mRecomputeWorldTransform = true;
		}
		void Move(const float x, const float y, const float z) noexcept
		{
			mPosition.x += x;
			mPosition.y += y;
			mPosition.z += z;
			mRecomputeWorldTransform = true;
		}
		const Vector3 &GetScale() const { return mScale; }
		void SetScale(const Vector3 &scale)
		{
			mScale = scale;
			mRecomputeWorldTransform = true;
		}
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
				ptr->AddedActor();
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
		void AddedScene() { addedSceneActorList = true; }

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
		bool addedSceneActorList = false;
	};
}
