#pragma once
#include "Scene.hpp"

class Main : public nen::Scene
{
public:
	Main() = default;
	~Main() = default;
	void Setup() override;
	void Update(float deltaTime) override;
};
