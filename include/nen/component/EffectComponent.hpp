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

		std::u16string GetPath() { return path; }

		const Vector3f &GetPosition() { return mOwner.GetPosition(); }

		void Play(bool play) { this->playing = play; }
		bool IsPlaying() { return this->playing; }

		void Stop() { isStop = true; }
		bool IsStop() { return isStop; }
		int handle;	

	private:
		Actor &mOwner;
		Effect *mEffect;
		std::u16string path;
		float mTimer;
		float buf;
		bool playing;
		bool isLoop;
		bool isStop;
	};
}
