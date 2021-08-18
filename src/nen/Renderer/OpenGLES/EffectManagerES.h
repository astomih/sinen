#pragma once
#include "../../effect/EffectManager.h"
namespace nen::es
{
    class EffectManagerES : public EffectManager
    {
    public:
        EffectManagerES(class ESRenderer *renderer);
        ~EffectManagerES() override
        {
        }
        void Init() override;

    private:
        class ESRenderer *mRenderer;
    };
}
