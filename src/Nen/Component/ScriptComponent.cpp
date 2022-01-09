#include <Nen.hpp>
#include <sol/sol.hpp>
namespace nen {
script_component::script_component(base_actor &owner, int updateOrder)
    : base_component(owner), m_script(script::Get()) {}
void script_component::DoScript(std::string_view funcName) {
  sol::state &lua = *m_script.GetSolState();

  auto &_actor = this->GetActor();
  lua["pos"] = this->GetActor().GetPosition();
  lua["rotate"] = this->GetActor().GetRotation();
  lua["setPosition"] = [&](vector3 pos) { _actor.SetPosition(pos); };
  lua["setRotation"] = [&](quaternion rot) { _actor.SetRotation(rot); };

  script::DoScript(funcName);
}
} // namespace nen
