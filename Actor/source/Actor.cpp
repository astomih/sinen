#include <Scene.hpp>
#include <Actors.hpp>
#include <Components.hpp>
#include <algorithm>
#include <Engine.hpp>

Actor::Actor(std::shared_ptr<Scene> scene)
	: mState(EActive), mPosition(Vector3f::Zero), mRotation(Quaternion::Identity), mScene(scene), mRecomputeWorldTransform(true), mScale(1.0f)
{
}

Actor::~Actor()
{
	mScene->RemoveActor(shared_from_this());
	// Need to delete components
	while (!mComponents.empty())
	{
		mComponents.pop_back();
	}
	mComponents2.clear();
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
	for (auto &comp : mComponents2)
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
		for (auto &comp : mComponents2)
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
		mRecomputeWorldTransform = true;

		// Scale, then rotate, then translate
		mWorldTransform =
			Matrix4::CreateTranslation(mPosition);
		/*
			* Matrix4::CreateScale(mScale)
			* Matrix4::CreateFromQuaternion(mRotation);
			*/

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
void Actor::AddComponent(std::unique_ptr<Component> &&component)
{
	// Find the insertion point in the sorted vector
	// (The first element with a order higher than me)
	int myOrder = component->GetUpdateOrder();
	auto iter = mComponents2.begin();
	for (; iter != mComponents2.end(); ++iter)
	{
		if (myOrder < (*iter)->GetUpdateOrder())
		{
			break;
		}
	}
	// Inserts element before position of iterator
	mComponents2.insert(iter, std::move(component));
}

void Actor::RemoveComponent(std::shared_ptr<Component> component)
{
	auto iter = std::find(mComponents.begin(), mComponents.end(), component);
	if (iter != mComponents.end())
	{
		mComponents.erase(iter);
	}
}

void Actor::Update(const std::shared_ptr<Scene> scene)
{
}