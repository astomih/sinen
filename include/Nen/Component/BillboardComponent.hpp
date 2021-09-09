#pragma once
#include "Component.hpp"

namespace nen
{
    class BillboardComponent : public Component
    {
    public:
        BillboardComponent(class Actor &owner, int updateOwner = 100);
        void Update(float deltaTime) override;
    };
}