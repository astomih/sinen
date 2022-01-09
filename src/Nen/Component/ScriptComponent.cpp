#include <Nen.hpp>
#include <sol/sol.hpp>
namespace nen {
script_component::script_component(base_actor &owner, int updateOrder)
    : base_component(owner), script(script::Get()) {}
void script_component::DoScript(std::string_view funcName) {
  sol::state &lua = *script.GetSolState();

  auto &actor = this->GetActor();
  lua["pos"] = this->GetActor().GetPosition();
  lua["rotate"] = this->GetActor().GetRotation();
  lua["setPosition"] = [&](vector3 pos) { actor.SetPosition(pos); };
  lua["setRotation"] = [&](quaternion rot) { actor.SetRotation(rot); };

  script::DoScript(funcName);
}
} // namespace nen
