#pragma once
#include <Engine.hpp>
namespace nen
{
	class Transition
	{
	public:
		enum class State : int16_t
		{
			BeforeStart,
			Processing,
			AfterEnd
		};
		Transition(State state = State::AfterEnd)
			:mState(state)
		{}
		State GetState() { return mState; }
		void ChangeState(const State& state) { mState = state; }
		bool NextState()
		{
			if (static_cast<int16_t>(mState) == 2)
				return false;
			mState = static_cast<State>(static_cast<int16_t>(mState) + 1);
			return true;
		}
		void Update();
		void Stop();

		double TimeBuf = 0.0;
	private:
		double mTime = 0.0;
		State mState;
	};
}
