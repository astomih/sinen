#include <Nen.hpp>
#include <algorithm>
#include <type_traits>
namespace nen
{
	Actor::Actor(Scene &scene)
		: mState(State::Active), mPosition(Vector3::Zero), mRotation(Quaternion::Identity), mScene(scene), mRecomputeWorldTransform(true), mScale(Vector3(1.f, 1.f, 1.f)), mComponents(), handle(0)
	{
	}

	Actor::~Actor()
	{
		mComponents.clear();
	}

	void Actor::UpdateActor(float deltaTime)
	{
		if (mState == State::Active)
		{
			for (auto comp : mComponents)
			{
				comp->Update(deltaTime);
			}
			Update(deltaTime);
			ComputeWorldTransform();
		}
	}

	void Actor::Update(float deltaTime)
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
	const InputState &Actor::GetInput() { return mScene.GetInput(); }
}