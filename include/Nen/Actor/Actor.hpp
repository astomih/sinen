#pragma once
#include "../Math/Matrix4.hpp"
#include "../Math/Vector3.hpp"
#include "../Math/Quaternion.hpp"
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
        enum State
        {
            EActive,
            EPaused,
            EDead
        };

        Actor(Scene &scene);
        virtual ~Actor();

        template <class T>
        std::shared_ptr<T> NewComponent(int updateOwder = 100)
        {
            if (std::is_base_of<Component, T>::value)
            {
                auto component = std::make_shared<T>(*this, updateOwder);
                this->AddComponent(component);
                return component;
            }
            else
                std::cerr << "NewComponent<T>:type T is not derived from Component." << std::endl;
        }
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

        void AddComponent(std::shared_ptr<class Component> component);
        void RemoveComponent(std::shared_ptr<class Component> component);
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
