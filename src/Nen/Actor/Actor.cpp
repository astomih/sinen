#include <Nen.hpp>
#include <algorithm>
#include <type_traits>
namespace nen
{
	Actor::Actor(Scene &scene)
		: mState(EActive), mPosition(Vector3::Zero), mRotation(Quaternion::Identity), mScene(scene), mRecomputeWorldTransform(true), mScale(Vector3(1.f, 1.f, 1.f)), mComponents()
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
}