#pragma once
#include "../nen.hpp"
namespace nen
{
	class EffectComponent : public Component
	{
	public:
		EffectComponent(class Actor &actor, std::u16string_view filepath);
		~EffectComponent();

		void Update(float deltaTime) override;

		void SetLoop(bool loop) { isLoop = loop; }
		bool IsLoop() { return isLoop; }

		void SetTimer(float timer)
		{
			mTimer = timer;
			buf = timer;
		}

		void Play(bool play) { this->playing = play; }
		bool IsPlaying() { return this->playing; }

		void Stop() { isStop = true; }
		bool IsStop() { return isStop; }

	private:
		Actor &mOwner;
		class Effect* mEffect;
		float mTimer;
		float buf;
		bool playing;
		bool isLoop;
		bool isStop;
	};
}
