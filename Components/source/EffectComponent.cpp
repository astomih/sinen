#include <Engine.hpp>
#include <Components.hpp>
#include <Scene.hpp>
#include <Actors.hpp>
#include <Common.hpp>
EffectComponent::EffectComponent(Actor &actor)
	: Component(actor)
{
	/*
	mOwner.GetScene()->GetRenderer()->AddEffectComp(this);
	mEffect = new Effect();
	mEffect->Init();
	mEffect->Load(u"Assets/Laser01.efk");
	*/
}

EffectComponent::~EffectComponent()
{
	//mOwner.GetScene()->GetRenderer()->RemoveEffectComp(this);
}

void EffectComponent::Draw()
{
	/*
	mEffect->manager->Update();
	mEffect->renderer->BeginRendering();
	mEffect->manager->Draw();
	mEffect->renderer->EndRendering();
	*/
}

void EffectComponent::Update(float deltaTime)
{
	/*
	auto r = mOwner.GetScene()->GetRenderer();
	mEffect->renderer->SetCameraMatrix(r->GetViewMatrix().ToEffekseer());
	mEffect->renderer->SetProjectionMatrix(r->GetProjectionMatrix().ToEffekseer());
	*/
}

void EffectComponent::Play(int32_t startFrame)
{
	/*
	auto common = mOwner.GetScene()->GetCommon();
	mEffect->handle = mEffect->manager->Play(mEffect->effect,
		Vector3f(common->x,common->y,common->z).ToEffekseer(), startFrame);
		*/
}
