#pragma once
#include <string>
namespace nen
{
	class EffectComponent : public Component
	{
	public:
		EffectComponent(class Actor &actor, int updateOrder = 100);
		~EffectComponent();

		void Create(std::u16string_view filePath);
		void Update(float deltaTime) override;

		void SetLoop(bool loop, Timer interval);
		bool isLoop();

	private:
		Actor &mOwner;
		std::shared_ptr<class Effect> mEffect;
	};
}
