#include <Nen.hpp>

namespace nen {
base_component::base_component(base_actor &owner, int updateOrder)
    : mOwner(owner), mUpdateOrder(updateOrder) {}

base_component::~base_component() {}

void base_component::Update(float deltaTime) {}
const input_state &base_component::GetInput() { return mOwner.GetInput(); }
} // namespace nen