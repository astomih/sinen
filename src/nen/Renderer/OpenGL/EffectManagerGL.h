#pragma once
#include "../../effect/EffectManager.h"
namespace nen::gl
{
	class EffectManagerGL : public EffectManager
	{
	public:
		EffectManagerGL(class GLRenderer *renderer);
		~EffectManagerGL() override
		{}
		void Init() override;

	private:
		class GLRenderer *mRenderer;
	};
}
