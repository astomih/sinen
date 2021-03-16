#pragma once
#include <Engine.hpp>
#include "Component.h"

class EffectComponent : public Component
{
public:
	EffectComponent(class Actor& actor);
	~EffectComponent();
	void Draw();
	void Update(float deltaTime) override;
	void Play(int32_t startFrame = 0);
private:
	Effect* mEffect;
};
