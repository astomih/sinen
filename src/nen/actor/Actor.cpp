#include <algorithm>
#include <nen.hpp>
#include <type_traits>
namespace nen
{
	Actor::Actor(Scene &scene)
		: mState(EActive), mPosition(Vector3f::Zero), mRotation(Quaternion::Identity), mScene(scene), mRecomputeWorldTransform(true), mScale(Vector3f(1.f, 1.f, 1.f)), mComponents()
	{
	}

	Actor::~Actor()
	{
		mComponents.clear();
	}

	void Actor::Update(float deltaTime)
	{
		if (mState == EActive)
		{
			UpdateComponents(deltaTime);
			UpdateActor(deltaTime);
			ComputeWorldTransform();
		}
	}

	void Actor::UpdateComponents(float deltaTime)
	{
		for (auto comp : mComponents)
		{
			comp->Update(deltaTime);
		}
	}

	void Actor::UpdateActor(float deltaTime)
	{
	}

	void Actor::ProcessInput(const InputState &state)
	{
		if (mState == EActive)
		{
			// First process input for components
			for (auto comp : mComponents)
			{
				comp->ProcessInput(state);
			}

			ActorInput(state);
		}
	}

	void Actor::ActorInput(const InputState &state)
	{
	}

	void Actor::ComputeWorldTransform()
	{
		if (mRecomputeWorldTransform)
		{
			this->RecomuteFinished();

			auto scaleOwner = GetScale();
			auto pos = GetPosition();
			Matrix4 t = Matrix4::Identity;
			t.mat[3][0] = pos.x;
			t.mat[3][1] = pos.y;
			t.mat[3][2] = pos.z;
			Matrix4 r = Matrix4::CreateFromQuaternion(this->GetRotation());
			Matrix4 s = Matrix4::Identity;
			s.mat[0][0] = scaleOwner.x;
			s.mat[1][1] = scaleOwner.y;
			s.mat[2][2] = scaleOwner.z;
			this->mWorldTransform = s * r * t;

			// Inform components world transform updated
			for (const auto comp : mComponents)
			{
				comp->OnUpdateWorldTransform();
			}
		}
	}

	void Actor::AddComponent(std::shared_ptr<Component> component)
	{
		// Find the insertion point in the sorted vector
		// (The first element with a order higher than me)
		int myOrder = component->GetUpdateOrder();
		if (!mComponents.empty())
		{
			auto iter = mComponents.begin();
			for (; iter != mComponents.end(); ++iter)
			{
				if (myOrder < (*iter)->GetUpdateOrder())
				{
					break;
				}
			}
			// Inserts element before position of iterator
			mComponents.insert(iter, component);
		}
		else
			mComponents.emplace_back(component);
		component->AddedActor();
	}

	void Actor::RemoveComponent(std::shared_ptr<Component> component)
	{
		auto iter = std::find(mComponents.begin(), mComponents.end(), component);
		if (iter != mComponents.end())
		{
			mComponents.erase(iter);
		}
	}
}