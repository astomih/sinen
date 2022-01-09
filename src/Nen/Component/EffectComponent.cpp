#include <Nen.hpp>
namespace nen {
effect_component::effect_component(base_actor &actor, int updateOrder)
    : base_component(actor, updateOrder), mOwner(actor) {}

effect_component::~effect_component() {
  mOwner.GetScene().GetRenderer()->RemoveEffect(mEffect);
  mEffect = nullptr;
}

void effect_component::Create(std::string_view filePath) {
  mEffect = std::make_shared<effect>(filePath);
  mOwner.GetScene().GetRenderer()->AddEffect(mEffect);
}
void effect_component::SetLoop(bool loop, timer interval) {
  mEffect->SetLoop(loop, interval);
}
bool effect_component::isLoop() { return mEffect->isLoop(); }

void effect_component::Update(float deltaTime) {
  if (mEffect) {
    auto pos = mOwner.GetPosition();
    mEffect->SetPosition(pos);

    mEffect->SetRotation(mOwner.GetRotation());
  }
}
} // namespace nen
