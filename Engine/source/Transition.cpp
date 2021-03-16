#include <Engine.hpp>
#include <SDL.h>

void Transition::Update()
{
	mState = State::Processing;
	mTime = SDL_GetTicks();
}

void Transition::Stop()
{
	mTime = SDL_GetTicks();
	mState = State::AfterEnd;
}