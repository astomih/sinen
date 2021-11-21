#pragma once
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <GL/glew.h>
#endif

namespace nen
{
	class EffectManager
	{
	public:
		EffectManager(){};
		virtual ~EffectManager() {}
		virtual void Init() {}
		virtual void Shutdown();

	protected:
	};
}